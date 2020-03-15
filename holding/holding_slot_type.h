#pragma once

namespace metternich {

enum class holding_slot_type
{
	none,
	settlement,
	palace,
	temple,
	fort,
	trading_post,
	hospital,
	university,
	factory
};

inline holding_slot_type string_to_holding_slot_type(const std::string &str)
{
	if (str == "settlement") {
		return holding_slot_type::settlement;
	} else if (str == "palace") {
		return holding_slot_type::palace;
	} else if (str == "temple") {
		return holding_slot_type::temple;
	} else if (str == "fort") {
		return holding_slot_type::fort;
	} else if (str == "trading_post") {
		return holding_slot_type::trading_post;
	} else if (str == "hospital") {
		return holding_slot_type::hospital;
	} else if (str == "university") {
		return holding_slot_type::university;
	} else if (str == "factory") {
		return holding_slot_type::factory;
	}

	throw std::runtime_error("Invalid holding slot type: \"" + str + "\".");
}

inline std::string holding_slot_type_to_string(const holding_slot_type type)
{
	switch (type) {
		case holding_slot_type::settlement:
			return "settlement";
		case holding_slot_type::palace:
			return "palace";
		case holding_slot_type::temple:
			return "temple";
		case holding_slot_type::fort:
			return "fort";
		case holding_slot_type::trading_post:
			return "trading_post";
		case holding_slot_type::hospital:
			return "hospital";
		case holding_slot_type::university:
			return "university";
		case holding_slot_type::factory:
			return "factory";
		default:
			break;
	}

	throw std::runtime_error("Invalid holding slot type: \"" + std::to_string(static_cast<int>(type)) + "\".");
}

inline bool is_holding_slot_type_string(const std::string &str)
{
	if (str == "settlement" || str == "palace" || str == "temple" || str == "fort" || str == "trading_post" || str == "hospital" || str == "university" || str == "factory") {
		return true;
	}

	return false;
}

inline bool is_extra_holding_slot_type(const holding_slot_type type)
{
	switch (type) {
		case holding_slot_type::temple:
		case holding_slot_type::fort:
		case holding_slot_type::trading_post:
		case holding_slot_type::hospital:
		case holding_slot_type::university:
		case holding_slot_type::factory:
			return true;
		default:
			return false;
	}
}

//whether the holding slot type is ownable on its, separate from its barony or province
inline bool is_separately_ownable_extra_holding_slot_type(const holding_slot_type type)
{
	switch (type) {
		case holding_slot_type::trading_post:
		case holding_slot_type::factory:
			return true;
		default:
			return false;
	}
}

}

Q_DECLARE_METATYPE(metternich::holding_slot_type)
