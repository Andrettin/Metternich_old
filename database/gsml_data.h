#pragma once

#include "database/gsml_property.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace metternich {

/**
**	@brief	Grand strategy markup language data
*/
class gsml_data
{
public:
	gsml_data(const std::string tag) : tag(tag) {}

	static gsml_data parse_file(const std::filesystem::path &filepath);

private:
	static std::vector<std::string> parse_line(const std::string &line);
	static bool parse_escaped_character(std::string &current_string, const char c);
	static void parse_tokens(const std::vector<std::string> &tokens, gsml_data **current_gsml_data);

public:
	const std::string &get_tag() const
	{
		return this->tag;
	}

	const gsml_data *get_parent() const
	{
		return this->parent;
	}

	const std::vector<gsml_data> &get_children() const
	{
		return this->children;
	}

	const std::vector<gsml_property> &get_properties() const
	{
		return this->properties;
	}

	const std::vector<std::string> &get_values() const
	{
		return this->values;
	}

	void sort_children()
	{
		//sort children by tag, alphabetically
		std::sort(this->children.begin(), this->children.end(), [](gsml_data &a, gsml_data &b) {
			return a.get_tag() < b.get_tag();
		});
	}

	void print(std::ofstream &ofstream) const
	{
		ofstream << this->get_tag() << " = {\n";

		for (const std::string &value : this->get_values()) {
			ofstream << value << " ";
		}

		for (const gsml_property &property : this->get_properties()) {
			property.print(ofstream);
		}

		for (const gsml_data &child_data : this->get_children()) {
			child_data.print(ofstream);
		}

		ofstream << "}\n";
	}

private:
	std::string tag;
	gsml_data *parent = nullptr;
	std::vector<gsml_data> children;
	std::vector<gsml_property> properties;
	std::vector<std::string> values; //values directly attached to the GSML data scope, used for e.g. name arrays
};

}
