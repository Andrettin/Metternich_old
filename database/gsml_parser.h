#pragma once

#include <filesystem>
#include <fstream>

namespace metternich {

class gsml_data;
enum class gsml_operator;

class gsml_parser
{
public:
	gsml_parser(const std::filesystem::path &filepath);

	gsml_data parse();

private:
	void parse_line(const std::string &line);
	bool parse_escaped_character(std::string &current_string, const char c);
	void parse_tokens();
	void reset();

private:
	std::filesystem::path filepath;
	std::vector<std::string> tokens;
	gsml_data *current_gsml_data = nullptr;
	std::string current_key;
	gsml_operator current_property_operator;
};

}
