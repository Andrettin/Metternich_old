#include "database/module.h"

#include "database/database.h"
#include "util/container_util.h"

namespace metternich {

void module::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

QVariantList module::get_dependencies_qvariant_list() const
{
	return container::to_qvariant_list(this->dependencies);
}

}
