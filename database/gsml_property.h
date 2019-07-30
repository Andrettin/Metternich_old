#pragma once

#include "database/gsml_operator.h"

#include <string>
#include <vector>

namespace Metternich {

class GSMLProperty
{
public:
	GSMLProperty(const std::string &key, GSMLOperator property_operator, const std::string &value) : Key(key), Operator(property_operator), Value(value)
	{
	}

	const std::string &GetKey() const
	{
		return this->Key;
	}

	GSMLOperator GetOperator() const
	{
		return this->Operator;
	}

	const std::string &GetValue() const
	{
		return this->Value;
	}

private:
	std::string Key;
	GSMLOperator Operator;
	std::string Value;
};

}
