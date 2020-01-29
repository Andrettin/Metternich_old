#include "species/species.h"

#include "translator.h"
#include "util/container_util.h"

namespace metternich {

std::string species::get_name_plural() const
{
	return translator::get()->translate(this->get_identifier_with_aliases(), {{"plural"}});
}

QVariantList species::get_evolutions_qvariant_list() const
{
	return container::to_qvariant_list(this->get_evolutions());
}

}
