#pragma once

#include "database/database.h"
#include "database/data_type_base.h"
#include "database/gsml_data.h"
#include "database/gsml_parser.h"

#include <filesystem>
#include <string>
#include <vector>

namespace metternich {

/**
**	@brief	The class for simple data types, which don't use identifiers
*/
template <typename T>
class simple_data_type : public data_type_base<T>
{
public:
	/**
	**	@brief	Parse the history database for the class
	*/
	static void parse_history_database()
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		for (const std::filesystem::path &path : database::get()->get_history_paths()) {
			std::filesystem::path history_path(path / T::database_folder);

			if (!std::filesystem::exists(history_path)) {
				continue;
			}

			database::parse_folder(history_path, T::gsml_history_data_to_process);
		}
	}
};

}
