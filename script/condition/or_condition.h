#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

/**
**	@brief	A scripted "or" condition
*/
class OrCondition : public Condition
{
public:
	virtual void ProcessGSMLProperty(const gsml_property &property) override
	{
		std::unique_ptr<Condition> condition = Condition::FromGSMLProperty(property);
		this->Conditions.push_back(std::move(condition));
	}

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "or";
		return identifier;
	}

	virtual bool Check(const Province *province) const override
	{
		return this->CheckInternal(province);
	}

	virtual bool Check(const holding *holding) const override
	{
		return this->CheckInternal(holding);
	}

private:
	template <typename T>
	bool CheckInternal(const T *scope) const
	{
		for (const std::unique_ptr<Condition> &condition : this->Conditions) {
			if (condition->Check(scope)) {
				return true;
			}
		}

		return false;
	}

private:
	std::vector<std::unique_ptr<Condition>> Conditions;
};

}
