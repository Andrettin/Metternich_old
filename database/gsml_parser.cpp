#include "database/gsml_parser.h"

#include "database/gsml_data.h"
#include "database/gsml_operator.h"

#include <fstream>
#include <stdexcept>

namespace metternich {

gsml_parser::gsml_parser(const std::filesystem::path &filepath)
	: filepath(filepath), current_property_operator(gsml_operator::none)
{
}

gsml_data gsml_parser::parse()
{
	if (!std::filesystem::exists(this->filepath)) {
		throw std::runtime_error("File \"" + this->filepath.string() + "\" not found.");
	}

	std::ifstream ifstream(this->filepath);

	if (!ifstream) {
		throw std::runtime_error("Failed to open file: " + this->filepath.string());
	}

	gsml_data file_gsml_data(this->filepath.stem().string());

	std::string line;
	int line_index = 1;
	this->current_gsml_data = &file_gsml_data;

	try {
		while (std::getline(ifstream, line)) {
			this->parse_line(line);
			this->parse_tokens();
			++line_index;
		}
	} catch (std::exception &exception) {
		throw std::runtime_error("Error parsing data file \"" + this->filepath.string() + "\", line " + std::to_string(line_index) + ": " + exception.what());
	}

	this->reset();

	return file_gsml_data;
}

void gsml_parser::parse_line(const std::string &line)
{
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
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				if (!current_string.empty()) {
					this->tokens.push_back(std::move(current_string));
					current_string = std::string();
				}

				continue;
			}
		}

		if (escaped) {
			escaped = false;

			if (this->parse_escaped_character(current_string, c)) {
				continue;
			}
		}

		current_string += c;
	}

	if (!current_string.empty()) {
		this->tokens.push_back(std::move(current_string));
	}
}

/**
**	@brief	Parse an escaped character in a GSML data file line
**
**	@param	current_string	The string currently being built from the parsing
**	@param	c				The character
**
**	@return	True if an escaped character was added to the string, or false otherwise
*/
bool gsml_parser::parse_escaped_character(std::string &current_string, const char c)
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
**	@brief	Parse the current tokens from the GSML data file
*/
void gsml_parser::parse_tokens()
{
	for (std::string &token : this->tokens) {
		if (!this->current_key.empty() && this->current_property_operator == gsml_operator::none && token != "=" && token != "+=" && token != "-=" && token != "==" && token != "!=" && token != "<" && token != "<=" && token != ">" && token != ">=" && token != "{") {
			//if the previously-given key isn't empty and no operator has been provided before or now, then the key was actually a value, part of a simple collection of values
			this->current_gsml_data->add_value(std::move(this->current_key));
			this->current_key = std::string();
		}

		if (this->current_key.empty()) {
			if (token == "{") { //opens a new, untagged scope
				gsml_data &new_gsml_data = this->current_gsml_data->add_child();
				new_gsml_data.parent = this->current_gsml_data;
				this->current_gsml_data = &new_gsml_data;
			} else if (token == "}") { //closes current tag
				if (this->current_gsml_data == nullptr) {
					throw std::runtime_error("Tried closing tag before any tag had been opened.");
				}

				if (this->current_gsml_data->get_parent() == nullptr) {
					throw std::runtime_error("Extra tag closing token!");
				}

				this->current_gsml_data = this->current_gsml_data->parent;
			} else { //key
				this->current_key = std::move(token);
			}

			continue;
		}

		if (this->current_property_operator == gsml_operator::none) { //operator
			if (token == "=") {
				this->current_property_operator = gsml_operator::assignment;
			} else if (token == "+=") {
				this->current_property_operator = gsml_operator::addition;
			} else if (token == "-=") {
				this->current_property_operator = gsml_operator::subtraction;
			} else if (token == "==") {
				this->current_property_operator = gsml_operator::equality;
			} else if (token == "!=") {
				this->current_property_operator = gsml_operator::inequality;
			} else if (token == "<") {
				this->current_property_operator = gsml_operator::less_than;
			} else if (token == "<=") {
				this->current_property_operator = gsml_operator::less_than_or_equality;
			} else if (token == ">") {
				this->current_property_operator = gsml_operator::greater_than;
			} else if (token == ">=") {
				this->current_property_operator = gsml_operator::greater_than_or_equality;
			} else {
				throw std::runtime_error("Tried using operator \"" + token + "\" for key \"" + this->current_key + "\", but it is not a valid operator.");
			}

			continue;
		}

		//value
		if (token == "{") { //opens tag
			if (this->current_property_operator != gsml_operator::assignment && this->current_property_operator != gsml_operator::addition) {
				throw std::runtime_error("Only the assignment and addition operators are valid after a tag.");
			}

			std::string &tag_name = this->current_key;
			gsml_data &new_gsml_data = this->current_gsml_data->add_child(std::move(tag_name), this->current_property_operator);
			new_gsml_data.parent = this->current_gsml_data;
			this->current_gsml_data = &new_gsml_data;
		} else {
			this->current_gsml_data->add_property(std::move(this->current_key), this->current_property_operator, std::move(token));
		}

		this->current_key = std::string();
		this->current_property_operator = gsml_operator::none;
	}

	this->tokens.clear();
}

void gsml_parser::reset()
{
	this->tokens.clear();
	this->current_gsml_data = nullptr;
	this->current_key = std::string();
	this->current_property_operator = gsml_operator::none;
}

}
