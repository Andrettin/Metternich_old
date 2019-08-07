#include "game/game.h"

#include "character/character.h"
#include "defines.h"
#include "game/game_speed.h"
#include "history/history.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "map/province.h"

#include <chrono>

namespace Metternich {

/**
**	@brief	Get the game singleton instance
*/
Game *Game::GetInstance()
{
	std::call_once(Game::OnceFlag, [](){ Game::Instance = std::make_unique<Game>(); });
	
	return Game::Instance.get();
}

/**
**	@brief	Constructor
*/
Game::Game() : Speed(GameSpeed::Normal)
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

	History::Load();

	this->GenerateMissingTitleHolders();
	this->PurgeSuperfluousCharacters();

	this->SetPlayerCharacter(Character::Get(Defines::GetPlayerCharacterID()));

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

void Game::DoTick()
{
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

void Game::DoDay()
{
}

void Game::DoMonth()
{
	for (Character *character : Character::GetAllLiving()) {
		character->DoMonth();
	}
}

void Game::DoYear()
{
}

/**
**	@brief	Generate holders for (non-titular) counties which lack them
*/
void Game::GenerateMissingTitleHolders()
{
	for (LandedTitle *landed_title : LandedTitle::GetAll()) {
		if (landed_title->GetHolder() != nullptr) {
			continue;
		}

		//generate missing title holders for county associated with provinces, or baronies associated with holdings
		if (landed_title->GetProvince() == nullptr && landed_title->GetHolding() == nullptr) {
			continue;
		}

		const Province *province = landed_title->GetProvince();
		if (province == nullptr) {
			province = landed_title->GetHolding()->GetProvince();
		}

		Character *holder = Character::Generate(province->GetCulture(), province->GetReligion());
		landed_title->SetHolder(holder);
	}
}

/**
**	@brief	Purge superfluous characters
*/
void Game::PurgeSuperfluousCharacters()
{
	std::vector<Character *> characters_to_remove;

	for (Character *character : Character::GetAll()) {
		//purge characters without a birth date, since this means that they were created during history loading, but haven't actually been born for the chosen start date (so that their birth wasn't loaded)
		if (!character->GetBirthDate().isValid()) {
			characters_to_remove.push_back(character);
		}
	}

	for (Character *character : characters_to_remove) {
		Character::Remove(character);
	}
}

}
