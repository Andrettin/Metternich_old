#include "database/module.h"

#include "database/database.h"
#include "util/container_util.h"
#include "util/translator.h"

namespace metternich {

std::string module::get_name() const
{
	return translator::get()->translate(this->get_identifier());
}

void module::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

QVariantList module::get_dependencies_qvariant_list() const
{
	return container::to_qvariant_list(this->dependencies);
}

}
