#include "game.h"

#include "game_speed.h"

#include <chrono>

Game *Game::GetInstance()
{
	std::call_once(Game::OnceFlag, [](){ Game::Instance = std::make_unique<Game>(); });
	
	return Game::Instance.get();
}

Game::Game() : Speed(GameSpeed::Normal)
{
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
