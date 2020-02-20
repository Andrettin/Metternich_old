#pragma once

namespace metternich {

enum class map_edge
{
	none,
	north,
	south,
	west,
	east,
	northwest,
	northeast,
	southwest,
	southeast
};

inline map_edge string_to_map_edge(const std::string &str)
{
	if (str == "none") {
		return map_edge::none;
	} else if (str == "north") {
		return map_edge::north;
	} else if (str == "south") {
		return map_edge::south;
	} else if (str == "west") {
		return map_edge::west;
	} else if (str == "east") {
		return map_edge::east;
	} else if (str == "northwest") {
		return map_edge::northwest;
	} else if (str == "northeast") {
		return map_edge::northeast;
	} else if (str == "southwest") {
		return map_edge::southwest;
	} else if (str == "southeast") {
		return map_edge::southeast;
	}

	throw std::runtime_error("Invalid map edge: \"" + str + "\".");
}

inline bool is_north_map_edge(const map_edge map_edge)
{
	switch (map_edge) {
		case map_edge::north:
		case map_edge::northwest:
		case map_edge::northeast:
			return true;
		default:
			return false;
	}
}

inline bool is_south_map_edge(const map_edge map_edge)
{
	switch (map_edge) {
		case map_edge::south:
		case map_edge::southwest:
		case map_edge::southeast:
			return true;
		default:
			return false;
	}
}

inline bool is_west_map_edge(const map_edge map_edge)
{
	switch (map_edge) {
		case map_edge::west:
		case map_edge::northwest:
		case map_edge::southwest:
			return true;
		default:
			return false;
	}
}

inline bool is_east_map_edge(const map_edge map_edge)
{
	switch (map_edge) {
		case map_edge::east:
		case map_edge::northeast:
		case map_edge::southeast:
			return true;
		default:
			return false;
	}
}

inline bool is_diagonal_map_edge(const map_edge map_edge)
{
	switch (map_edge) {
		case map_edge::northwest:
		case map_edge::northeast:
		case map_edge::southwest:
		case map_edge::southeast:
			return true;
		default:
			return false;
	}
}

}

Q_DECLARE_METATYPE(metternich::map_edge)
