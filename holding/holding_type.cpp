#include "holding/holding_type.h"

#include "database/gsml_data.h"
#include "politics/law.h"
#include "script/modifier.h"
#include "util/container_util.h"
#include "util/map_util.h"

namespace metternich {

std::set<std::string> holding_type::get_database_dependencies()
{
	return {
		//so that laws will have their groups when added to the default laws map
		law::class_identifier
	};
}

holding_type::holding_type(const std::string &identifier) : data_entry(identifier)
{
}

holding_type::~holding_type()
{
}

void holding_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::modifier<holding>>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

QVariantList holding_type::get_default_laws_qvariant_list() const
{
	return container::to_qvariant_list(map::get_values(this->default_laws));
}

Q_INVOKABLE void holding_type::add_default_law(law *law)
{
	this->default_laws[law->get_group()] = law;
}

Q_INVOKABLE void holding_type::remove_default_law(law *law)
{
	this->default_laws.erase(law->get_group());
}

}
