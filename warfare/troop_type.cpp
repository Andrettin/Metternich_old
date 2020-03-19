#include "warfare/troop_type.h"

#include "util/translator.h"
#include "warfare/troop_category.h"

namespace metternich {

troop_type::troop_type(const std::string &identifier)
	: data_entry(identifier), category(troop_category::none)
{
}

void troop_type::check() const
{
	if (this->get_category() == troop_category::none) {
		throw std::runtime_error("Troop type \"" + this->get_identifier() + "\" has no category.");
	}

	this->get_icon_path(); //throws an exception if the icon isn't found
}

std::string troop_type::get_category_name() const
{
	return translator::get()->translate(troop_category_to_string(this->get_category()));
}

const std::filesystem::path &troop_type::get_icon_path() const
{
	std::string base_tag = this->get_icon_tag();
	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

}
