#pragma once

#include "database/gsml_data.h"

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Metternich {

/**
**	@brief	The base class for the data type class per se, and for simpler data type classes which don't use identifiers
*/
template <typename T>
class DataTypeBase
{
protected:
	static inline std::vector<GSMLData> GSMLHistoryDataToProcess;
};

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

			T::GSMLHistoryDataToProcess.push_back(GSMLData::ParseFile(dir_entry.path()));
		}
	}
};

}
