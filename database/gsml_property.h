#pragma once

#include <string>
#include <vector>

namespace metternich {

enum class gsml_operator : int;

class gsml_property
{
public:
	gsml_property(std::string &&key, const gsml_operator property_operator, std::string &&value)
		: key(std::move(key)), property_operator(property_operator), value(std::move(value))
	{
	}

	gsml_property(const std::string &key, const gsml_operator property_operator, const std::string &value)
		: key(key), property_operator(property_operator), value(value)
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

	void print(std::ofstream &ofstream, const size_t indentation) const;

private:
	std::string key;
	gsml_operator property_operator;
	std::string value;
};

}
