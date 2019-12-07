#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/world.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

class landed_title;
enum class landed_title_tier;

template <typename T, landed_title_tier TIER>
class tier_de_jure_title_condition : public condition<T>
{
public:
	tier_de_jure_title_condition(const std::string &title_identifier)
	{
		this->tier_de_jure_title = landed_title::get(title_identifier);

		if (this->tier_de_jure_title->get_tier() != TIER) {
			throw std::runtime_error("\"" + this->tier_de_jure_title->get_identifier() + "\" is not a valid \"" + landed_title::get_tier_identifier(TIER) + "\" tier landed title.");
		}
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "tier_de_jure_title";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		const landed_title *title = get_scope_landed_title(scope);
		return title != nullptr && title->get_tier_de_jure_title(TIER) == this->tier_de_jure_title;
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		const landed_title *title = get_scope_landed_title(scope);

		if (title != nullptr) {
			title->connect(title, &landed_title::de_jure_liege_title_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
		}
	}

private:
	const landed_title *tier_de_jure_title = nullptr;
};

}
