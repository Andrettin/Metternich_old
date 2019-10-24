#pragma once

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

		std::filesystem::path history_path("./data/history/" + std::string(T::database_folder));

		if (!std::filesystem::exists(history_path)) {
			return;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(history_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			gsml_parser parser(dir_entry.path());
			T::gsml_history_data_to_process.push_back(parser.parse());
		}
	}
};

}
