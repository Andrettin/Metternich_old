#include "economy/commodity.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/chance_factor.h"

namespace metternich {

commodity::commodity(const std::string &identifier) : data_entry(identifier)
{
}

commodity::~commodity()
{
}

void commodity::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "chance") {
		this->chance = std::make_unique<chance_factor<holding_slot>>();
		database::process_gsml_data(this->chance, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

const std::filesystem::path &commodity::get_icon_path() const
{
	std::string base_tag = this->get_icon_tag();
	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

}
