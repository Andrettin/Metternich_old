#pragma once

#include "database/gsml_operator.h"

#include <filesystem>
#include <fstream>

namespace metternich {

class gsml_data;

class gsml_parser
{
public:
	gsml_parser(const std::filesystem::path &filepath) : filepath(filepath)
	{
	}

	gsml_data parse();

private:
	void parse_line(const std::string &line);
	bool parse_escaped_character(std::string &current_string, const char c);
	void parse_tokens();

	void reset()
	{
		this->tokens.clear();
		this->current_gsml_data = nullptr;
		this->key.clear();
		this->property_operator = gsml_operator::none;
		this->value.clear();
	}

private:
	std::filesystem::path filepath;
	std::vector<std::string> tokens;
	gsml_data *current_gsml_data = nullptr;
	std::string key;
	gsml_operator property_operator = gsml_operator::none;
	std::string value;
};

}
