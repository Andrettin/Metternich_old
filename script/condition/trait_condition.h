#pragma once

#include "character/trait.h"
#include "script/condition/condition.h"

#include <type_traits>

namespace metternich {

class trait;

template <typename T>
class trait_condition : public condition<T>
{
public:
	trait_condition(const std::string &trait_identifier)
	{
		this->trait = trait::get(trait_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "trait";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->has_trait(this->trait);
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		scope->connect(scope, &T::traits_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	const trait *trait = nullptr;
};

}
