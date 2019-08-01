#include "game.h"

#include "character.h"
#include "game_speed.h"
#include "history/history.h"
#include "landed_title.h"
#include "map/province.h"

#include <chrono>

namespace Metternich {

Game *Game::GetInstance()
{
	std::call_once(Game::OnceFlag, [](){ Game::Instance = std::make_unique<Game>(); });
	
	return Game::Instance.get();
}

Game::Game() : Speed(GameSpeed::Normal)
{
}

void Game::Start(const QDateTime &start_date)
{
	this->CurrentDate = start_date;
	emit CurrentDateChanged();

	History::Load();

	this->GenerateMissingTitleHolders();

	this->Running = true;
	emit RunningChanged();

	std::thread game_loop_thread(&Game::Run, this);
	game_loop_thread.detach();
}

void Game::Run()
{
	while (true) {
		std::chrono::time_point<std::chrono::system_clock> tick_start = std::chrono::system_clock::now();

		this->CurrentDate = this->CurrentDate.addDays(1);
		emit CurrentDateChanged();

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

void Game::GenerateMissingTitleHolders()
{
	for (LandedTitle *landed_title : LandedTitle::GetAll()) {
		if (landed_title->GetHolder() != nullptr) {
			continue;
		}

		const Province *province = landed_title->GetProvince();
		if (province == nullptr) {
			continue;
		}

		Character *holder = Character::Generate(province->GetCulture(), province->GetReligion());
		landed_title->SetHolder(holder);
	}
}

}
