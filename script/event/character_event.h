#pragma once

#include "database/data_type.h"
#include "script/event/event.h"
#include "script/event/scoped_event_base.h"

namespace metternich {

class character;

class character_event : public event, public data_type<character_event>, public scoped_event_base<character>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "character_event";
	static constexpr const char *database_base_folder = "events";
	static constexpr const char *database_folder = "character_events";

	character_event(const std::string &identifier) : event(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();

		if (tag == "conditions" || tag == "option") {
			scoped_event_base::process_gsml_scope(scope);
		} else {
			data_entry_base::process_gsml_scope(scope);
		}
	}
};

}
