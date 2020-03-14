#pragma once

namespace metternich {

enum class map_mode
{
	none,
	country,
	de_jure_empire,
	de_jure_kingdom,
	de_jure_duchy,
	culture,
	culture_group,
	religion,
	religion_group,
	trade_node,
	trade_zone //the map mode detailing which countries control trading posts where
};

}
