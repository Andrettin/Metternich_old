#pragma once

#include "database/gsml_operator.h"

#include <string>
#include <vector>

namespace metternich {

class gsml_property
{
public:
	gsml_property(const std::string &key, gsml_operator property_operator, const std::string &value) : key(key), property_operator(property_operator), value(value)
	{
	}

	const std::string &get_key() const
	{
		return this->key;
	}

	gsml_operator get_operator() const
	{
		return this->property_operator;
	}

	const std::string &get_value() const
	{
		return this->value;
	}

	void print(std::ofstream &ofstream, const size_t indentation) const
	{
		ofstream << std::string(indentation, '\t') << this->get_key() << " ";

		switch (this->get_operator()) {
			case gsml_operator::assignment:
				ofstream << "=";
				break;
			case gsml_operator::addition:
				ofstream << "+=";
				break;
			case gsml_operator::subtraction:
				ofstream << "-=";
				break;
			case gsml_operator::none:
				throw std::runtime_error("Cannot print the GSML \"none\" operator.");
		}

		ofstream << " " << this->get_value() << "\n";
	}

private:
	std::string key;
	gsml_operator property_operator;
	std::string value;
};

}
