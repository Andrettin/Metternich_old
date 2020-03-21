#include "technology/technology_area.h"

#include "technology/technology.h"
#include "technology/technology_area_compare.h"
#include "technology/technology_category.h"
#include "util/container_util.h"
#include "util/translator.h"
#include "util/vector_util.h"

namespace metternich {

std::vector<technology_area *> technology_area::get_all_sorted()
{
	std::vector<technology_area *> technology_areas = technology_area::get_all();
	std::sort(technology_areas.begin(), technology_areas.end(), technology_area_compare());
	return technology_areas;
}

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

QVariantList technology_area::get_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->technologies);
}

void technology_area::remove_technology(technology *technology)
{
	vector::remove(this->technologies, technology);
}


int technology_area::get_min_level() const
{
	if (this->technologies.empty()) {
		return 0;
	}

	//get the lowest level amongst the technologies for the area
	int level = std::numeric_limits<int>::max();

	for (technology *technology : this->technologies) {
		level = std::min(level, technology->get_level());
	}

	return level;
}

}
