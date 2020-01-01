#pragma once

namespace metternich {

enum class government_type_group
{
	monarchy,
	republic,
	theocracy,
	tribe
};

inline government_type_group string_to_government_type_group(const std::string &str)
{
	if (str == "monarchy") {
		return government_type_group::monarchy;
	} else if (str == "republic") {
		return government_type_group::republic;
	} else if (str == "theocracy") {
		return government_type_group::theocracy;
	} else if (str == "tribe") {
		return government_type_group::tribe;
	}

	throw std::runtime_error("Invalid government type group: \"" + str + "\".");
}

inline std::string government_type_group_to_string(const government_type_group group)
{
	switch (group) {
		case government_type_group::monarchy:
			return "monarchy";
		case government_type_group::republic:
			return "republic";
		case government_type_group::theocracy:
			return "theocracy";
		case government_type_group::tribe:
			return "tribe";
	}

	throw std::runtime_error("Invalid government type group: \"" + std::to_string(static_cast<int>(group)) + "\".");
}

}

Q_DECLARE_METATYPE(metternich::government_type_group)
