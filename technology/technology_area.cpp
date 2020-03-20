#include "technology/technology_area.h"

#include "technology/technology.h"
#include "technology/technology_category.h"
#include "util/translator.h"
#include "util/vector_util.h"

namespace metternich {

technology_area::technology_area(const std::string &identifier)
	: data_entry(identifier), category(technology_category::none)
{
}

void technology_area::check() const
{
	if (this->get_category() == technology_category::none) {
		throw std::runtime_error("Technology area \"" + this->get_identifier() + "\" has no category.");
	}
}

std::string technology_area::get_category_name() const
{
	return translator::get()->translate(technology_category_to_string(this->get_category()));
}

void technology_area::remove_technology(technology *technology)
{
	vector::remove(this->technologies, technology);
}


int technology_area::get_min_level() const
{
	//get the lowest level amongst the technologies for the area
	int level = 0;

	for (technology *technology : this->technologies) {
		level = std::min(level, technology->get_level());
	}

	return level;
}

}
