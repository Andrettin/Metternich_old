#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

/**
**	@brief	A scripted "and" condition
*/
class AndCondition : public Condition
{
public:
	virtual void process_gsml_property(const gsml_property &property) override
	{
		std::unique_ptr<Condition> condition = Condition::FromGSMLProperty(property);
		this->Conditions.push_back(std::move(condition));
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "and";
		return identifier;
	}

	virtual bool check(const Province *province) const override
	{
		return this->check_internal(province);
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check_internal(holding);
	}

private:
	template <typename T>
	bool check_internal(const T *scope) const
	{
		for (const std::unique_ptr<Condition> &condition : this->Conditions) {
			if (!condition->check(scope)) {
				return false;
			}
		}

		return true;
	}

private:
	std::vector<std::unique_ptr<Condition>> Conditions;
};

}
