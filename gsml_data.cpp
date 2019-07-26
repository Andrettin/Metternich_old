#include "gsml_data.h"

#include "gsml_operator.h"

#include <fstream>
#include <stdexcept>

/**
**	@brief	Parse a GSML data file
**
**	@param	filepath	The path to the file holding the GSML data
**
**	@return	The parsed GSML data for the file.
*/
GSMLData GSMLData::ParseFile(const std::filesystem::path &filepath)
{
	if (!std::filesystem::exists(filepath)) {
		throw std::runtime_error("File \"" + filepath.string() + "\" not found.");
	}

	std::ifstream ifstream(filepath);

	if (!ifstream) {
		throw std::runtime_error("Failed to open file: " + filepath.string());
	}

	GSMLData file_gsml_data;

	std::string line;
	int line_index = 1;
	GSMLData *current_gsml_data = &file_gsml_data;
	while (std::getline(ifstream, line)) {
		try {
			std::vector<std::string> tokens = GSMLData::ParseLine(line);
			GSMLData::ParseTokens(tokens, &current_gsml_data);
		} catch (std::exception &exception) {
			throw std::runtime_error("Error parsing data file \"" + filepath.string() + "\", line " + std::to_string(line_index) + ": " + exception.what() + ".");
		}

		++line_index;
	}

	if (file_gsml_data.Children.empty()) {
		throw std::runtime_error("Could not parse output for config file \"" + filepath.string() + "\".");
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
std::vector<std::string> GSMLData::ParseLine(const std::string &line)
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
					tokens.push_back(std::string(1, c));
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

			if (GSMLData::ParseEscapedCharacter(current_string, c)) {
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
bool GSMLData::ParseEscapedCharacter(std::string &current_string, const char c)
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
void GSMLData::ParseTokens(const std::vector<std::string> &tokens, GSMLData **current_gsml_data)
{
	std::string key;
	GSMLOperator property_operator = GSMLOperator::None;
	std::string value;
	for (const std::string &token : tokens) {
		if (key.empty()) {
			if (token == "}") { //closes current tag
				if ((*current_gsml_data) == nullptr) {
					throw std::runtime_error("Tried closing tag before any tag had been opened.");
				}

				if ((*current_gsml_data)->Parent == nullptr) {
					throw std::runtime_error("Extra tag closing token!");
				}

				(*current_gsml_data) = (*current_gsml_data)->Parent;
			} else { //key
				key = token;
			}

			continue;
		}

		if (property_operator == GSMLOperator::None) { //operator
			if (token == "=") {
				property_operator = GSMLOperator::Assignment;
			} else if (token == "+=") {
				property_operator = GSMLOperator::Addition;
			} else if (token == "-=") {
				property_operator = GSMLOperator::Subtraction;
			} else {
				throw std::runtime_error("Tried using operator \"" + token + "\" for key \"" + key + "\", but it is not a valid operator.");
			}

			continue;
		}

		//value
		if (token == "{") { //opens tag
			if (property_operator != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is valid after a tag!");
			}

			std::string tag_name = key;
			(*current_gsml_data)->Children.emplace_back(tag_name);
			GSMLData &new_gsml_data = (*current_gsml_data)->Children.back();
			new_gsml_data.Parent = *current_gsml_data;
			(*current_gsml_data) = &new_gsml_data;
		} else {
			(*current_gsml_data)->Properties.push_back(GSMLProperty(key, property_operator, token));
		}

		key.clear();
		property_operator = GSMLOperator::None;
	}
}
