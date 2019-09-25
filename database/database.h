#pragma once

#include "database/gsml_data.h"
#include "singleton.h"
#include "type_traits.h"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace metternich {

class data_entry;

/**
**	@brief	The database
*/
class database : public singleton<database>
{
public:
	template <typename T>
	static void process_gsml_data(T *instance, const gsml_data &data)
	{
		for (const gsml_property &property : data.get_properties()) {
			instance->process_gsml_property(property);
		}

		for (const gsml_data &child_data : data.get_children()) {
			instance->process_gsml_scope(child_data);
		}
	}

	template <typename T>
	static void process_gsml_data(T &instance, const gsml_data &data)
	{
		if constexpr (is_specialization_of_v<T, std::unique_ptr>) {
			database::process_gsml_data(instance.get(), data);
		} else {
			database::process_gsml_data(&instance, data);
		}
	}

	static void process_gsml_property_for_object(QObject *object, const gsml_property &property);

public:
	void load();

	void initialize_history()
	{
		//initialize data entries are valid for each data type
		for (const std::function<void()> &function : this->history_initialization_functions) {
			function();
		}

		//check if data entries are valid for each data type
		for (const std::function<void()> &function : this->checking_functions) {
			function();
		}
	}

	void add_parsing_function(const std::function<void()> &function)
	{
		this->parsing_functions.push_back(function);
	}

	void add_processing_function(const std::function<void(bool)> &function)
	{
		this->processing_functions.push_back(function);
	}

	void add_checking_function(const std::function<void()> &function)
	{
		this->checking_functions.push_back(function);
	}

	void add_initialization_function(const std::function<void()> &function)
	{
		this->initialization_functions.push_back(function);
	}

	void add_history_initialization_function(const std::function<void()> &function)
	{
		this->history_initialization_functions.push_back(function);
	}

private:
	std::vector<std::function<void()>> parsing_functions; //parsing functions for each data type
	std::vector<std::function<void(bool)>> processing_functions; //processing functions for each data type
	std::vector<std::function<void()>> checking_functions; //functions for each data type, to check if data entries are valid
	std::vector<std::function<void()>> initialization_functions; //functions for each data type, to initialize their entries
	std::vector<std::function<void()>> history_initialization_functions; //functions for each data type, to initialize their entries' history
};

}
