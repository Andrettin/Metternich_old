#pragma once

#include "database/gsml_operator.h"

#include <string>
#include <vector>

namespace metternich {

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

	void Print(std::ofstream &ofstream) const
	{
		ofstream << this->GetKey() << " ";

		switch (this->GetOperator()) {
			case GSMLOperator::Assignment:
				ofstream << "=";
			case GSMLOperator::Addition:
				ofstream << "+=";
			case GSMLOperator::Subtraction:
				ofstream << "-=";
			case GSMLOperator::None:
				throw std::runtime_error("Cannot print the GSML \"none\" operator.");
		}

		ofstream << " " << this->GetValue() << "\n";
	}

private:
	std::string Key;
	GSMLOperator Operator;
	std::string Value;
};

}
