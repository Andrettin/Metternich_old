#pragma once

namespace metternich {

enum class landed_title_tier : int
{
	barony, //holding
	county, //province
	duchy,
	kingdom,
	empire,
	cosmic_barony, //continent/world-level holding
	cosmic_county, //world
	cosmic_duchy, //star system
	cosmic_kingdom,
	cosmic_empire
};

inline bool is_cosmic_landed_title_tier(const landed_title_tier tier)
{
	switch (tier) {
		case landed_title_tier::cosmic_barony:
		case landed_title_tier::cosmic_county:
		case landed_title_tier::cosmic_duchy:
		case landed_title_tier::cosmic_kingdom:
		case landed_title_tier::cosmic_empire:
			return true;
		default:
			return false;
	}
}

}
