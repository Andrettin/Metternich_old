#include "game/game.h"

#include "character/character.h"
#include "defines.h"
#include "game/game_speed.h"
#include "game/tick_period.h"
#include "history/history.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/map_mode.h"
#include "map/province.h"
#include "map/world.h"
#include "script/condition/condition_check_base.h"
#include "script/event/event_trigger.h"

#include <chrono>

namespace metternich {

game::game() : speed(game_speed::normal), tick_period(tick_period::day)
{
}

void game::start(const timeline *timeline, const QDateTime &start_date)
{
	this->starting = true;
	this->speed = defines::get()->get_default_game_speed();

	this->total_ticks = 0;
	history::get()->set_timeline(timeline);
	history::get()->set_start_date(start_date);
	this->current_date = start_date;
	emit current_date_changed();

	history::get()->load();

	this->generate_missing_title_holders();
	this->amalgamate_map_inactive_worlds();
	this->purge_superfluous_characters();

	if (defines::get()->get_player_character_title()->get_holder() != nullptr) {
		this->set_player_character(defines::get()->get_player_character_title()->get_holder());
	} else {
		throw std::runtime_error("No valid player character.");
	}

	map::get()->set_mode(map_mode::country);
	this->set_tick_period(tick_period::day);

	this->starting = false;
	this->running = true;
	emit running_changed();
	this->set_paused(true);

	for (character *character : character::get_all_living()) {
		character_event_trigger::game_start->do_events(character);
	}

	std::thread game_loop_thread(&game::run, this);
	game_loop_thread.detach();
}

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

void game::do_tick()
{
	condition_check_base::recalculate_pending_checks();

	//process the orders given by the player
	this->do_orders();

	if (this->is_paused()) {
		return;
	}

	this->total_ticks++;

	const QDateTime old_date = current_date;

	switch (this->tick_period) {
		case tick_period::millenium:
			this->current_date = this->current_date.addYears(1000);
			break;
		case tick_period::day:
			this->current_date = this->current_date.addDays(1);
			break;
	}

	emit current_date_changed();

	switch (this->tick_period) {
		case tick_period::millenium:
			//if the tick period is less than a day, do yearly actions (e.g. character aging) once every 365 ticks nevertheless
			if (this->total_ticks % 365 == 0) {
				this->do_year();
			}
			break;
		case tick_period::day:
			if (old_date.date().year() != this->current_date.date().year()) {
				this->do_year();
			}

			if (old_date.date().month() != this->current_date.date().month()) {
				this->do_month();
			}

			if (old_date.date().day() != this->current_date.date().day()) {
				this->do_day();
			}

			break;
	}
}

void game::do_day()
{
	const QDate date = this->current_date.date();
	const size_t days_in_month = static_cast<size_t>(date.daysInMonth());
	const size_t days_in_year = static_cast<size_t>(date.daysInYear());
	const size_t current_day = static_cast<size_t>(date.day());
	const size_t current_year_day = static_cast<size_t>(date.dayOfYear());

	const std::vector<holding_slot *> &holding_slots = holding_slot::get_all();
	//process the monthly actions of different ones on each day of the month, for the sake of performance
	for (size_t i = (current_day - 1); i < holding_slots.size(); i += days_in_month) {
		holding_slot *holding_slot = holding_slots[i];

		if (holding_slot->get_holding() == nullptr) {
			continue;
		}

		holding_slot->get_holding()->do_month();
	}

	for (holding_slot *holding_slot : holding_slots) {
		if (holding_slot->get_holding() == nullptr) {
			continue;
		}

		holding_slot->get_holding()->do_day();
	}

	const std::vector<province *> &provinces = province::get_all();
	for (size_t i = (current_day - 1); i < provinces.size(); i += days_in_month) {
		province *province = provinces[i];

		if (province->get_county() == nullptr) {
			continue;
		}

		province->do_month();
	}

	for (province *province : provinces) {
		if (province->get_county() == nullptr) {
			continue;
		}

		province->do_day();
	}

	const std::vector<character *> &living_characters = character::get_all_living();
	for (size_t i = (current_year_day - 1); i < living_characters.size(); i += days_in_year) {
		character *character = living_characters[i];

		if (character == nullptr) {
			continue;
		}

		character->do_year();
	}

	for (size_t i = (current_day - 1); i < living_characters.size(); i += days_in_month) {
		character *character = living_characters[i];
		if (character == nullptr) {
			continue;
		}

		character->do_month();
	}
}

void game::do_month()
{
}

void game::do_year()
{
	//dead characters were replaced by nullptrs in the living character list to not upset the monthly/yearly pulse order, but when a year turns they can be freely purged
	character::purge_null_characters();
}

QString game::get_current_date_string() const
{
	QLocale english_locale(QLocale::English);
	QString current_date_string;

	if (this->tick_period == tick_period::day) {
		current_date_string += english_locale.toString(this->current_date, "d MMMM, ");
	}

	const int year = this->current_date.date().year();
	const int abs_year = std::abs(year);
	if (abs_year >= 10000) {
		current_date_string += english_locale.toString(abs_year);
	} else {
		current_date_string += QString::number(abs_year);
	}

	if (year < 0) {
		current_date_string += " BC";
	} else {
		current_date_string += " AD";
	}

	return current_date_string;
}

void game::set_player_character(character *character)
{
	if (character == this->get_player_character()) {
		return;
	}

	metternich::character *old_character = this->get_player_character();

	this->player_character = character;
	emit player_character_changed();

	if (old_character != nullptr) {
		emit old_character->ai_changed();
	}

	if (character != nullptr) {
		emit character->ai_changed();
	}
}

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

		//generate missing title holders for counties associated with territories, or baronies associated with holdings
		const territory *territory = landed_title->get_territory();
		const star_system *system = landed_title->get_star_system();
		if (
			(territory == nullptr || territory->get_settlement_holdings().empty())
			&& landed_title->get_holding() == nullptr
			&& (system == nullptr || landed_title->get_capital_world()->get_settlement_holdings().empty())
		) {
			continue;
		}

		culture *culture = nullptr;
		religion *religion = nullptr;

		if (territory != nullptr) {
			culture = territory->get_culture();
			religion = territory->get_religion();
		} else if (landed_title->get_holding() != nullptr) {
			culture = landed_title->get_holding()->get_culture();
			religion = landed_title->get_holding()->get_religion();
		} else if (system != nullptr) {
			culture = landed_title->get_capital_world()->get_culture();
			religion = landed_title->get_capital_world()->get_religion();
		}

		character *holder = character::generate(culture, religion);
		landed_title->set_holder(holder);

		if (landed_title->get_holding() != nullptr) {
			//set the county holder as the liege of the generated owner for the holding
			character *county_holder = landed_title->get_holding()->get_territory()->get_county()->get_holder();
			holder->set_liege(county_holder);
		} else if (landed_title->get_world() != nullptr) {
			//set the duchy holder as the liege of the generated owner for the world
			character *duchy_holder = territory->get_duchy()->get_holder();
			holder->set_liege(duchy_holder);
		}
	}
}

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

void game::amalgamate_map_inactive_worlds()
{
	//for worlds which have a map that has become inactive, amalgamate them
	for (world *world : world::get_map_worlds()) {
		if (world->is_map_active()) {
			continue;
		}

		world->amalgamate();
	}
}

}
