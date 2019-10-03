#include "game/game.h"

#include "character/character.h"
#include "defines.h"
#include "game/game_speed.h"
#include "history/history.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "map/province.h"

#include <chrono>

namespace metternich {

/**
**	@brief	Constructor
*/
Game::Game() : Speed(GameSpeed::Fast)
{
}

/**
**	@brief	Start the game
**
**	@param	start_date	The game's start date
*/
void Game::Start(const QDateTime &start_date)
{
	this->Starting = true;

	this->CurrentDate = start_date;
	emit CurrentDateChanged();

	History::load();

	this->generate_missing_title_holders();
	this->PurgeSuperfluousCharacters();

	this->SetPlayerCharacter(Character::get(Defines::get()->GetPlayerCharacterID()));

	this->Starting = false;
	this->Running = true;
	emit RunningChanged();

	std::thread game_loop_thread(&Game::Run, this);
	game_loop_thread.detach();
}

/**
**	@brief	Run the game's game loop
*/
void Game::Run()
{
	while (true) {
		std::chrono::time_point<std::chrono::system_clock> tick_start = std::chrono::system_clock::now();

		this->DoTick();

		std::chrono::time_point<std::chrono::system_clock> tick_end = std::chrono::system_clock::now();

		std::chrono::milliseconds tick_run_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tick_end - tick_start);

		//sleep for the remaining duration of the tick, if any
		std::chrono::milliseconds tick_ms;

		switch (this->Speed) {
			case GameSpeed::Slowest: {
				tick_ms = std::chrono::milliseconds(2000);
				break;
			}
			case GameSpeed::Slow: {
				tick_ms = std::chrono::milliseconds(1000);
				break;
			}
			case GameSpeed::Normal: {
				tick_ms = std::chrono::milliseconds(500);
				break;
			}
			case GameSpeed::Fast: {
				tick_ms = std::chrono::milliseconds(100);
				break;
			}
			case GameSpeed::Fastest: {
				tick_ms = std::chrono::milliseconds(1); //about as fast as possible, but leaving a bit of time for the UI thread to process its event loop
				break;
			}
		}

		tick_ms -= tick_run_ms;
		if (tick_ms > std::chrono::milliseconds(0)) {
			std::this_thread::sleep_for(tick_ms);
		}
	}
}

/**
**	@brief	Do the game's per tick actions
*/
void Game::DoTick()
{
	//process the orders given by the player
	this->DoOrders();

	QDateTime old_date = CurrentDate;
	this->CurrentDate = this->CurrentDate.addDays(1);
	emit CurrentDateChanged();

	if (old_date.date().day() != this->CurrentDate.date().day()) {
		this->DoDay();
	}

	if (old_date.date().month() != this->CurrentDate.date().month()) {
		this->DoMonth();
	}

	if (old_date.date().year() != this->CurrentDate.date().year()) {
		this->DoYear();
	}
}

/**
**	@brief	Do the game's daily actions
*/
void Game::DoDay()
{
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
void Game::DoMonth()
{
	for (province *province : province::get_all()) {
		if (province->get_county() == nullptr) {
			continue;
		}

		province->do_month();
	}

	for (Character *character : Character::get_all_living()) {
		character->DoMonth();
	}
}

/**
**	@brief	Do the game's yearly actions
*/
void Game::DoYear()
{
}

/**
**	@brief	Generate holders for (non-titular) counties which lack them
*/
void Game::generate_missing_title_holders()
{
	std::vector<LandedTitle *> landed_titles = LandedTitle::get_all();
	std::sort(landed_titles.begin(), landed_titles.end(), [](const LandedTitle *a, const LandedTitle *b) {
		//give priority to landed titles with greater rank (so that counties will be processed before baronies)
		return a->GetTier() > b->GetTier();
	});

	for (LandedTitle *landed_title : landed_titles) {
		if (landed_title->GetHolder() != nullptr) {
			continue;
		}

		//generate missing title holders for county associated with provinces, or baronies associated with holdings
		if (landed_title->get_province() == nullptr && landed_title->get_holding() == nullptr) {
			continue;
		}

		const province *province = landed_title->get_province();
		if (province == nullptr) {
			province = landed_title->get_holding()->get_province();
		}

		Character *holder = Character::generate(province->get_culture(), province->get_religion());
		landed_title->SetHolder(holder);

		//set the liege of generated holding owners to the county holder
		if (landed_title->get_holding() != nullptr) {
			Character *county_holder = landed_title->get_holding()->get_province()->get_county()->GetHolder();
			holder->SetLiege(county_holder);
		}
	}
}

/**
**	@brief	Purge superfluous characters
*/
void Game::PurgeSuperfluousCharacters()
{
	std::vector<Character *> characters_to_remove;

	for (Character *character : Character::get_all()) {
		//purge characters without a birth date, since this means that they were created during history loading, but haven't actually been born for the chosen start date (so that their birth wasn't loaded)
		if (!character->GetBirthDate().isValid()) {
			characters_to_remove.push_back(character);
		}
	}

	for (Character *character : characters_to_remove) {
		Character::remove(character);
	}
}

}
