#pragma once

namespace metternich {

enum class holding_slot_type : int
{
	settlement,
	palace,
	temple,
	fort,
	trading_post,
	hospital,
	university
};

inline holding_slot_type string_to_holding_slot_type(const std::string str)
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
	}

	throw std::runtime_error("Invalid holding slot type: \"" + str + "\".");
}

inline bool is_extra_holding_slot_type(const holding_slot_type type)
{
	switch (type) {
		case holding_slot_type::temple:
		case holding_slot_type::fort:
		case holding_slot_type::trading_post:
		case holding_slot_type::hospital:
		case holding_slot_type::university:
			return true;
		default:
			return false;
	}
}

}

Q_DECLARE_METATYPE(metternich::holding_slot_type)
