#pragma once

#include "database/data_type.h"
#include "script/decision/decision.h"
#include "script/decision/scoped_decision.h"

namespace metternich {

class holding;

class holding_decision : public decision, public data_type<holding_decision>, public scoped_decision<holding>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "holding_decision";
	static constexpr const char *database_base_folder = "decisions";
	static constexpr const char *database_folder = "holding_decisions";

	holding_decision(const std::string &identifier) : decision(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();

		if (tag == "preconditions" || tag == "conditions" || tag == "source_preconditions" || tag == "source_conditions" || tag == "effects") {
			scoped_decision::process_gsml_scope(scope);
		} else {
			data_entry_base::process_gsml_scope(scope);
		}
	}

	bool check_conditions(const holding *holding, const character *source) const;
	Q_INVOKABLE bool check_conditions(const QVariant &holding_variant, const QVariant &source_variant) const;
	Q_INVOKABLE void do_effects(const QVariant &holding_variant, const QVariant &source_variant) const;
	Q_INVOKABLE QString get_effects_string(const QVariant &holding_variant) const;
};

}
