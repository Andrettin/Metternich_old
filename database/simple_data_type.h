#pragma once

#include "database/data_type_base.h"
#include "database/gsml_data.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Metternich {

/**
**	@brief	The class for simple data types, which don't use identifiers
*/
template <typename T>
class SimpleDataType : public DataTypeBase<T>
{
public:
	/**
	**	@brief	Parse the history database for the class
	*/
	static void ParseHistoryDatabase()
	{
		std::filesystem::path history_path("./data/history/" + std::string(T::DatabaseFolder));

		if (!std::filesystem::exists(history_path)) {
			return;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(history_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			T::gsml_history_data_to_process.push_back(gsml_data::parse_file(dir_entry.path()));
		}
	}
};

}
