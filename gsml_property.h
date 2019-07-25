#pragma once

#include "gsml_operator.h"

#include <string>
#include <vector>

class GSMLProperty
{
public:
	GSMLProperty(const std::string &key, GSMLOperator property_operator, const std::string &value) : Key(key), Operator(property_operator), Value(value)
	{
	}

private:
	std::string Key;
	GSMLOperator Operator;
	std::string Value;
};
