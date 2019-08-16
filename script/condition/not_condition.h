#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace Metternich {

/**
**	@brief	A scripted "not" condition
*/
class NotCondition : public Condition
{
public:
	virtual void ProcessGSMLProperty(const GSMLProperty &property) override
	{
		std::unique_ptr<Condition> condition = Condition::FromGSMLProperty(property);
		this->Conditions.push_back(std::move(condition));
	}

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "not";
		return identifier;
	}

	virtual bool Check(const Holding *holding) const override
	{
		for (const std::unique_ptr<Condition> &condition : this->Conditions) {
			if (condition->Check(holding)) {
				return false;
			}
		}

		return true;
	}

private:
	std::vector<std::unique_ptr<Condition>> Conditions;
};

}
