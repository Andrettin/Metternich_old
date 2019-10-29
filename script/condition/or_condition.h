#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

/**
**	@brief	A scripted "or" condition
*/
class or_condition : public condition
{
public:
	virtual void process_gsml_property(const gsml_property &property) override
	{
		std::unique_ptr<condition> condition = condition::from_gsml_property(property);
		this->conditions.push_back(std::move(condition));
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "or";
		return identifier;
	}

	virtual bool check(const province *province) const override
	{
		return this->check_internal(province);
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check_internal(holding);
	}

	virtual bool check(const holding_slot *holding_slot) const override
	{
		return this->check_internal(holding_slot);
	}

private:
	template <typename T>
	bool check_internal(const T *scope) const
	{
		for (const std::unique_ptr<condition> &condition : this->conditions) {
			if (condition->check(scope)) {
				return true;
			}
		}

		return false;
	}

private:
	std::vector<std::unique_ptr<condition>> conditions;
};

}
