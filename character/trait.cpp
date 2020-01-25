#include "character/trait.h"

#include "script/modifier.h"

namespace metternich {

trait::trait(const std::string &identifier) : data_entry(identifier)
{
}

trait::~trait()
{
}

void trait::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::modifier<character>>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

const std::filesystem::path &trait::get_icon_path() const
{
	std::string base_tag = this->get_icon_tag();
	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

}
