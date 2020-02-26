#pragma once

namespace metternich {

enum class landed_title_tier : int
{
	barony, //holding
	county, //province
	duchy,
	kingdom,
	empire,
	world_empire,
	star_empire, //star system
	interstellar_empire,
	galactic_empire
};

inline bool is_cosmic_landed_title_tier(const landed_title_tier tier)
{
	switch (tier) {
		case landed_title_tier::world_empire:
		case landed_title_tier::star_empire:
		case landed_title_tier::interstellar_empire:
		case landed_title_tier::galactic_empire:
			return true;
		default:
			return false;
	}
}

}
