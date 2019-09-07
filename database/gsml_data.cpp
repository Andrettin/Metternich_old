#include "database/gsml_data.h"

#include "database/gsml_operator.h"

#include <fstream>
#include <stdexcept>

namespace metternich {

/**
**	@brief	Parse a GSML data file
**
**	@param	filepath	The path to the file holding the GSML data
**
**	@return	The parsed GSML data for the file.
*/
gsml_data gsml_data::parse_file(const std::filesystem::path &filepath)
{
	if (!std::filesystem::exists(filepath)) {
		throw std::runtime_error("File \"" + filepath.string() + "\" not found.");
	}

	std::ifstream ifstream(filepath);

	if (!ifstream) {
		throw std::runtime_error("Failed to open file: " + filepath.string());
	}

	gsml_data file_gsml_data(filepath.stem().string());

	std::string line;
	int line_index = 1;
	gsml_data *current_gsml_data = &file_gsml_data;
	std::vector<std::string> tokens;

	try {
		while (std::getline(ifstream, line)) {
				std::vector<std::string> new_tokens = gsml_data::parse_line(line);
				for (const std::string &token : new_tokens) {
					tokens.push_back(token);
				}

			++line_index;
		}

		gsml_data::parse_tokens(tokens, &current_gsml_data);
	} catch (std::exception &exception) {
		throw std::runtime_error("Error parsing data file \"" + filepath.string() + "\", line " + std::to_string(line_index) + ": " + exception.what() + ".");
	}

	return file_gsml_data;
}

/**
**	@brief	Parse a line in a GSML data file
**
**	@param	line	The line to be parsed
**
**	@return	A vector holding the line's tokens
*/
std::vector<std::string> gsml_data::parse_line(const std::string &line)
{
	std::vector<std::string> tokens;

	bool opened_quotation_marks = false;
	bool escaped = false;
	std::string current_string;

	for (const char c : line) {
		if (!escaped) {
			if (c == '\"') {
				opened_quotation_marks = !opened_quotation_marks;
				continue;
			} else if (c == '\\') {
				escaped = true; //escape character, so that e.g. newlines can be properly added to text
				continue;
			}
		}

		if (!opened_quotation_marks) {
			if (c == '#') {
				break; //ignore what is written after the comment symbol ('#'), as well as the symbol itself, unless it occurs within quotes
			}

			//whitespace, carriage returns and etc. separate tokens, if they occur outside of quotes
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '=') {
				if (c == '=') { //the assignment operator separates tokens, but is also a token in and of itself
					std::string operator_token;
					if (!current_string.empty()) {
						if (current_string.back() == '+' || current_string.back() == '-') {
							operator_token = std::string(1, current_string.back());
							current_string.pop_back();
						}
					}
					operator_token += std::string(1, c);
					tokens.push_back(operator_token);
				}

				if (!current_string.empty()) {
					tokens.push_back(current_string);
					current_string.clear();
				}

				continue;
			}
		}

		if (escaped) {
			escaped = false;

			if (gsml_data::parse_escaped_character(current_string, c)) {
				continue;
			}
		}

		current_string += c;
	}

	if (!current_string.empty()) {
		tokens.push_back(current_string);
	}

	return tokens;
}

/**
**	@brief	Parse an escaped character in a GSML data file line
**
**	@param	current_string	The string currently being built from the parsing
**	@param	c				The character
**
**	@return	True if an escaped character was added to the string, or false otherwise
*/
bool gsml_data::parse_escaped_character(std::string &current_string, const char c)
{
	if (c == 'n') {
		current_string += '\n';
	} else if (c == 't') {
		current_string += '\t';
	} else if (c == 'r') {
		current_string += '\r';
	} else if (c == '\"') {
		current_string += '\"';
	} else if (c == '\\') {
		current_string += '\\';
	} else {
		return false;
	}

	return true;
}

/**
**	@brief	Parse the tokens from a GSML data file line
**
**	@param	tokens				The tokens to be parsed
**	@param	current_gsml_data	The current GSML data element being processed
*/
void gsml_data::parse_tokens(const std::vector<std::string> &tokens, gsml_data **current_gsml_data)
{
	std::string key;
	gsml_operator property_operator = gsml_operator::none;
	std::string value;
	for (const std::string &token : tokens) {
		if (!key.empty() && property_operator == gsml_operator::none && token != "=" && token != "+=" && token != "-=" && token != "{") {
			//if the previously-given key isn't empty and no operator has been provided before or now, then the key was actually a value, part of a simple collection of values
			(*current_gsml_data)->values.push_back(key);
			key.clear();
		}

		if (key.empty()) {
			if (token == "}") { //closes current tag
				if ((*current_gsml_data) == nullptr) {
					throw std::runtime_error("Tried closing tag before any tag had been opened.");
				}

				if ((*current_gsml_data)->parent == nullptr) {
					throw std::runtime_error("Extra tag closing token!");
				}

				(*current_gsml_data) = (*current_gsml_data)->parent;
			} else { //key
				key = token;
			}

			continue;
		}

		if (property_operator == gsml_operator::none) { //operator
			if (token == "=") {
				property_operator = gsml_operator::assignment;
			} else if (token == "+=") {
				property_operator = gsml_operator::addition;
			} else if (token == "-=") {
				property_operator = gsml_operator::subtraction;
			} else {
				throw std::runtime_error("Tried using operator \"" + token + "\" for key \"" + key + "\", but it is not a valid operator.");
			}

			continue;
		}

		//value
		if (token == "{") { //opens tag
			if (property_operator != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator is valid after a tag!");
			}

			std::string tag_name = key;
			(*current_gsml_data)->children.emplace_back(tag_name);
			gsml_data &new_gsml_data = (*current_gsml_data)->children.back();
			new_gsml_data.parent = *current_gsml_data;
			(*current_gsml_data) = &new_gsml_data;
		} else {
			(*current_gsml_data)->properties.push_back(gsml_property(key, property_operator, token));
		}

		key.clear();
		property_operator = gsml_operator::none;
	}
}

}
