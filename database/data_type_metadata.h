#pragma once

namespace metternich {

/**
**	@brief	The metadata for a data type, including e.g. its initialization function
*/
class data_type_metadata
{
public:
	data_type_metadata(const std::string &class_identifier, const std::set<std::string> &database_dependencies, const std::function<void()> &parsing_function, const std::function<void(bool)> &processing_function, const std::function<void()> &checking_function, const std::function<void()> &initialization_function, const std::function<void()> &history_initialization_function)
		: class_identifier(class_identifier), database_dependencies(database_dependencies), parsing_function(parsing_function), processing_function(processing_function), checking_function(checking_function), initialization_function(initialization_function), history_initialization_function(history_initialization_function)
	{
	}

	const std::string &get_class_identifier() const
	{
		return this->class_identifier;
	}

	bool has_database_dependency_on(const std::string &class_identifier) const
	{
		return this->database_dependencies.find(class_identifier) != this->database_dependencies.end();
	}

	bool has_database_dependency_on(const std::unique_ptr<data_type_metadata> &metadata) const
	{
		return this->has_database_dependency_on(metadata->get_class_identifier());
	}

	size_t get_database_dependency_count() const
	{
		return this->database_dependencies.size();
	}

	const std::function<void()> &get_parsing_function() const
	{
		return this->parsing_function;
	}

	const std::function<void(bool)> &get_processing_function() const
	{
		return this->processing_function;
	}

	const std::function<void()> &get_checking_function() const
	{
		return this->checking_function;
	}

	const std::function<void()> &get_initialization_function() const
	{
		return this->initialization_function;
	}

	const std::function<void()> &get_history_initialization_function() const
	{
		return this->history_initialization_function;
	}

private:
	std::string class_identifier;
	std::set<std::string> database_dependencies;
	std::function<void()> parsing_function; //parsing functions for each data type
	std::function<void(bool)> processing_function; //processing functions for each data type
	std::function<void()> checking_function; //functions for each data type, to check if data entries are valid
	std::function<void()> initialization_function; //functions for each data type, to initialize their entries
	std::function<void()> history_initialization_function; //functions for each data type, to initialize their entries' history
};

}
