#pragma once

namespace metternich {

enum class game_speed : int
{
	slowest,
	slow,
	normal,
	fast,
	fastest
};

inline game_speed string_to_game_speed(const std::string &str)
{
	if (str == "slowest") {
		return game_speed::slowest;
	} else if (str == "slow") {
		return game_speed::slow;
	} else if (str == "normal") {
		return game_speed::normal;
	} else if (str == "fast") {
		return game_speed::fast;
	} else if (str == "fastest") {
		return game_speed::fastest;
	}

	throw std::runtime_error("Invalid game speed: \"" + str + "\".");
}

inline std::string game_speed_to_string(const game_speed speed)
{
	switch (speed) {
		case game_speed::slowest:
			return "slowest";
		case game_speed::slow:
			return "slow";
		case game_speed::normal:
			return "normal";
		case game_speed::fast:
			return "fast";
		case game_speed::fastest:
			return "fastest";
	}

	throw std::runtime_error("Invalid game speed: \"" + std::to_string(static_cast<int>(speed)) + "\".");
}

}
