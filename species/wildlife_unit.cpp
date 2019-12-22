#include "species/wildlife_unit.h"

#include "map/province.h"
#include "map/region.h"
#include "species/species.h"

#include <QApplication>

namespace metternich {

void wildlife_unit::process_history_database()
{
	//simple data types are only loaded in history, instanced directly based on their GSML data
	for (const gsml_data &data : wildlife_unit::gsml_history_data_to_process) {
		for (const gsml_data &data_entry : data.get_children()) {
			const std::string &species_identifier = data_entry.get_tag();
			metternich::species *species = species::get(species_identifier);
			auto wildlife_unit = std::make_unique<metternich::wildlife_unit>(species);
			wildlife_unit->moveToThread(QApplication::instance()->thread());
			wildlife_unit->process_history(data_entry);
			wildlife_unit->load_history();

			if (wildlife_unit->get_size() <= 0) {
				continue; //don't add empty wildlife units
			}

			if (wildlife_unit->get_province() != nullptr) {
				wildlife_unit->get_province()->add_wildlife_unit(std::move(wildlife_unit));
			} else if (wildlife_unit->get_region() != nullptr) {
				wildlife_unit->get_region()->add_wildlife_unit(std::move(wildlife_unit));
			} else {
				throw std::runtime_error("Wildlife unit of species \"" + species_identifier + "\" belongs to neither a province, nor a region.");
			}
		}
	}

	wildlife_unit::gsml_history_data_to_process.clear();
}

void wildlife_unit::check_history() const
{
	if (this->get_province()->get_owner() == nullptr && this->get_clade() == nullptr) {
		throw std::runtime_error("Wildlife unit of species \"" + this->get_species()->get_identifier() + "\" has no clade, but lives in a province without a sapient owner.");
	}
}

void wildlife_unit::do_month()
{
	if (this->get_size() == 0) {
		return;
	}
}

clade *wildlife_unit::get_clade() const
{
	return this->get_species()->get_clade();
}

void wildlife_unit::subtract_existing_sizes()
{
	if (this->get_province() != nullptr) {
		this->subtract_existing_sizes_in_province(this->get_province());
	} else if (this->get_region() != nullptr) {
		this->subtract_existing_sizes_in_provinces(this->get_region()->get_provinces());
	}
}

void wildlife_unit::subtract_existing_sizes_in_province(const metternich::province *province)
{
	for (const std::unique_ptr<wildlife_unit> &wildlife_unit : province->get_wildlife_units()) {
		if (wildlife_unit.get() == this) {
			continue;
		}

		if (wildlife_unit->get_species() != this->get_species()) {
			continue;
		}

		this->change_size(-wildlife_unit->get_size());
	}
}

void wildlife_unit::subtract_existing_sizes_in_provinces(const std::set<metternich::province *> &provinces)
{
	for (const metternich::province *province : provinces) {
		this->subtract_existing_sizes_in_province(province);
	}
}

bool wildlife_unit::can_distribute_to_province(const metternich::province *province) const
{
	if (this->discounts_existing()) {
		//the wildlife unit can only be distributed to the given province if there is no wildlife unit there with the same species as this one, if discount existing is enabled
		for (const std::unique_ptr<wildlife_unit> &wildlife_unit : province->get_wildlife_units()) {
			if (wildlife_unit->get_species() != this->get_species()) {
				continue;
			}

			return false;
		}
	}

	return true;
}

void wildlife_unit::distribute_to_provinces(const std::set<metternich::province *> &provinces)
{
	//set population for provinces without population data for that species
	int province_count = 0; //count of province for which the population will be applied

	for (const metternich::province *province : provinces) {
		if (this->can_distribute_to_province(province)) {
			province_count++;
		}
	}

	if (province_count == 0) {
		return;
	}

	//now, apply the remaining population to all provinces without population of that species set for them, in equal proportions
	const int size_per_province = this->get_size() / province_count;

	if (size_per_province <= 0) {
		return;
	}

	for (metternich::province *province : provinces) {
		if (!this->can_distribute_to_province(province)) {
			continue;
		}

		auto wildlife_unit = std::make_unique<metternich::wildlife_unit>(this->get_species());
		wildlife_unit->moveToThread(QApplication::instance()->thread());
		wildlife_unit->set_province(province);
		wildlife_unit->set_size(size_per_province);
		province->add_wildlife_unit(std::move(wildlife_unit));
	}
}

const std::filesystem::path &wildlife_unit::get_icon_path() const
{
	const std::string &base_tag = this->get_species()->get_icon_tag();
	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

}
