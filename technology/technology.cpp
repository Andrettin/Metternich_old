#include "technology/technology.h"

#include "script/modifier.h"
#include "util/container_util.h"
#include "util/vector_util.h"

namespace metternich {

technology::technology(const std::string &identifier)
	: data_entry(identifier)
{
}

technology::~technology()
{
}

void technology::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "holding_modifier") {
		this->holding_modifier = std::make_unique<modifier<holding>>();
		database::process_gsml_data(this->holding_modifier, scope);
	} else if (tag == "territory_modifier") {
		this->territory_modifier = std::make_unique<modifier<territory>>();
		database::process_gsml_data(this->territory_modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

const std::filesystem::path &technology::get_icon_path() const
{
	const std::string &base_tag = this->get_icon_tag();

	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

QVariantList technology::get_required_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->get_required_technologies());
}

void technology::remove_required_technology(technology *technology)
{
	this->required_technologies.erase(technology);
	vector::remove(technology->allowed_technologies, this);
}

QVariantList technology::get_allowed_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->allowed_technologies);
}

}
