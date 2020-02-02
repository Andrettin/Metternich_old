#pragma once

#include "script/condition/and_condition.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

class province;

template <typename T>
class location_condition : public condition<T>
{
public:
	location_condition(const gsml_operator effect_operator) : condition<T>(effect_operator)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		this->conditions.process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->conditions.process_gsml_scope(scope);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "location";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		const province *location = scope->get_location();
		if (location == nullptr) {
			return false;
		}

		return this->conditions.check(location);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::location_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);

		const province *location = scope->get_location();
		if (location != nullptr) {
			this->conditions.bind_condition_check(check, location);
		}
	}

private:
	and_condition<province> conditions;
};

}
