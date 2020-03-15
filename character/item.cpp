#include "character/item.h"

#include "script/modifier.h"
#include "util/string_util.h"
#include "util/translator.h"

namespace metternich {

item::item(const std::string &identifier) : data_entry(identifier)
{
}

item::~item()
{
}

void item::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::modifier<character>>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

std::string item::get_description() const
{
	return translator::get()->translate(this->get_identifier() + "_desc");
}

const std::filesystem::path &item::get_icon_path() const
{
	std::string base_tag = this->get_icon_tag();
	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

QString item::get_modifier_effects_string() const
{
	if (this->get_modifier() == nullptr) {
		return QString();
	}

	return string::to_tooltip(this->get_modifier()->get_string());
}

}
