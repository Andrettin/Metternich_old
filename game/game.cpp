#include "game/game.h"

#include "character/character.h"
#include "defines.h"
#include "game/game_speed.h"
#include "history/history.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "script/condition/condition_check_base.h"

#include <chrono>

namespace metternich {

/**
**	@brief	Constructor
*/
game::game() : speed(game_speed::fast)
{
}

/**
**	@brief	Start the game
**
**	@param	start_date	The game's start date
*/
void game::start(const timeline *timeline, const QDateTime &start_date)
{
	this->starting = true;

	history::get()->set_timeline(timeline);
	history::get()->set_start_date(start_date);
	this->current_date = start_date;
	emit current_date_changed();

	history::get()->load();

	this->generate_missing_title_holders();
	this->purge_superfluous_characters();

	this->set_player_character(character::get(defines::get()->get_player_character_id()));

	this->starting = false;
	this->running = true;
	emit running_changed();

	std::thread game_loop_thread(&game::run, this);
	game_loop_thread.detach();
}

/**
**	@brief	Run the game's game loop
*/
void game::run()
{
	while (!this->should_stop) {
		std::chrono::time_point<std::chrono::system_clock> tick_start = std::chrono::system_clock::now();

		this->do_tick();

		std::chrono::time_point<std::chrono::system_clock> tick_end = std::chrono::system_clock::now();

		std::chrono::milliseconds tick_run_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tick_end - tick_start);

		//sleep for the remaining duration of the tick, if any
		std::chrono::milliseconds tick_ms;

		switch (this->speed) {
			case game_speed::slowest: {
				tick_ms = std::chrono::milliseconds(2000);
				break;
			}
			case game_speed::slow: {
				tick_ms = std::chrono::milliseconds(1000);
				break;
			}
			case game_speed::normal: {
				tick_ms = std::chrono::milliseconds(500);
				break;
			}
			case game_speed::fast: {
				tick_ms = std::chrono::milliseconds(100);
				break;
			}
			case game_speed::fastest: {
				tick_ms = std::chrono::milliseconds(1); //about as fast as possible, but leaving a bit of time for the UI thread to process its event loop
				break;
			}
		}

		tick_ms -= tick_run_ms;
		if (tick_ms > std::chrono::milliseconds(0)) {
			std::this_thread::sleep_for(tick_ms);
		}
	}

	this->running = false;
	this->should_stop = false;
}

/**
**	@brief	Do the game's per tick actions
*/
void game::do_tick()
{
	condition_check_base::recalculate_pending_checks();

	//process the orders given by the player
	this->do_orders();

	QDateTime old_date = current_date;
	this->current_date = this->current_date.addDays(1);
	emit current_date_changed();

	if (old_date.date().day() != this->current_date.date().day()) {
		this->do_day();
	}

	if (old_date.date().month() != this->current_date.date().month()) {
		this->do_month();
	}

	if (old_date.date().year() != this->current_date.date().year()) {
		this->do_year();
	}
}

/**
**	@brief	Do the game's daily actions
*/
void game::do_day()
{
	for (holding_slot *holding_slot : holding_slot::get_all()) {
		if (holding_slot->get_holding() == nullptr) {
			continue;
		}

		holding_slot->get_holding()->do_day();
	}

	for (province *province : province::get_all()) {
		if (province->get_county() == nullptr) {
			continue;
		}

		province->do_day();
	}
}

/**
**	@brief	Do the game's monthly actions
*/
void game::do_month()
{
	for (holding_slot *holding_slot : holding_slot::get_all()) {
		if (holding_slot->get_holding() == nullptr) {
			continue;
		}

		holding_slot->get_holding()->do_month();
	}

	for (province *province : province::get_all()) {
		if (province->get_county() == nullptr) {
			continue;
		}

		province->do_month();
	}

	for (character *character : character::get_all_living()) {
		character->do_month();
	}
}

/**
**	@brief	Do the game's yearly actions
*/
void game::do_year()
{
}

/**
**	@brief	Generate holders for (non-titular) counties which lack them
*/
void game::generate_missing_title_holders()
{
	std::vector<landed_title *> landed_titles = landed_title::get_all();
	std::sort(landed_titles.begin(), landed_titles.end(), [](const landed_title *a, const landed_title *b) {
		//give priority to landed titles with greater rank (so that counties will be processed before baronies)
		return a->get_tier() > b->get_tier();
	});

	for (landed_title *landed_title : landed_titles) {
		if (landed_title->get_holder() != nullptr) {
			continue;
		}

		//generate missing title holders for county associated with provinces, or baronies associated with holdings
		if ((landed_title->get_province() == nullptr || landed_title->get_province()->get_settlement_holdings().empty()) && landed_title->get_holding() == nullptr) {
			continue;
		}

		culture *culture = nullptr;
		religion *religion = nullptr;

		if (landed_title->get_province() != nullptr) {
			culture = landed_title->get_province()->get_culture();
			religion = landed_title->get_province()->get_religion();
		} else if (landed_title->get_holding() != nullptr) {
			culture = landed_title->get_holding()->get_culture();
			religion = landed_title->get_holding()->get_religion();
		}

		character *holder = character::generate(culture, religion);
		landed_title->set_holder(holder);

		//set the county holder as the liege of the generated owner for the holding
		if (landed_title->get_holding() != nullptr) {
			character *county_holder = landed_title->get_holding()->get_province()->get_county()->get_holder();
			holder->set_liege(county_holder);
		}
	}
}

/**
**	@brief	Purge superfluous characters
*/
void game::purge_superfluous_characters()
{
	std::vector<character *> characters_to_remove;

	for (character *character : character::get_all()) {
		//purge characters without a birth date, since this means that they were created during history loading, but haven't actually been born for the chosen start date (so that their birth wasn't loaded)
		if (!character->get_birth_date().isValid()) {
			characters_to_remove.push_back(character);
		}
	}

	for (character *character : characters_to_remove) {
		character::remove(character);
	}
}

}
