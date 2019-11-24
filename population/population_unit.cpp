#include "population/population_unit.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "economy/employment.h"
#include "economy/employment_type.h"
#include "game/game.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "map/province.h"
#include "map/region.h"
#include "phenotype.h"
#include "population/population_type.h"
#include "random.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "util/container_util.h"

#include <QApplication>

namespace metternich {

/**
**	@brief	Process the history database for population units
*/
void population_unit::process_history_database()
{
	//simple data types are only loaded in history, instanced directly based on their GSML data
	for (const gsml_data &data : population_unit::gsml_history_data_to_process) {
		for (const gsml_data &data_entry : data.get_children()) {
			const std::string type_identifier = data_entry.get_tag();
			population_type *type = population_type::get(type_identifier);
			auto population_unit = std::make_unique<metternich::population_unit>(type);
			population_unit->moveToThread(QApplication::instance()->thread());
			population_unit->load_history(const_cast<gsml_data &>(data_entry));

			if (population_unit->get_size() <= 0) {
				continue; //don't add empty population units
			}

			if (population_unit->get_holding() != nullptr) {
				population_unit->get_holding()->add_population_unit(std::move(population_unit));
			} else if (population_unit->get_province() != nullptr) {
				population_unit->get_province()->add_population_unit(std::move(population_unit));
			} else if (population_unit->get_region() != nullptr) {
				population_unit->get_region()->add_population_unit(std::move(population_unit));
			} else {
				throw std::runtime_error("Population unit of type \"" + type_identifier + "\" belongs to neither a holding, nor a province, nor a region.");
			}
		}
	}

	population_unit::gsml_history_data_to_process.clear();
}

/**
**	@brief	Initialize the population unit's history
*/
void population_unit::initialize_history()
{
	//set the culture and religion of population units without any set to those of their holding
	if (this->get_culture() == nullptr) {
		this->set_culture(this->get_holding()->get_culture());
	}

	if (this->get_religion() == nullptr) {
		this->set_religion(this->get_holding()->get_religion());
	}
}

/**
**	@brief	Do the population unit's monthly actions
*/
void population_unit::do_month()
{
	if (this->get_size() == 0) {
		return;
	}

	if (this->get_unemployed_size() > 0) {
		this->seek_employment();
	}

	this->do_mixing();
}

/**
**	@brief	Do mixing for the population unit
*/
void population_unit::do_mixing()
{
	for (const std::unique_ptr<population_unit> &other_population_unit : this->get_holding()->get_population_units()) {
		if (other_population_unit.get() == this) {
			continue;
		}

		if (other_population_unit->get_type() != this->get_type() || other_population_unit->get_culture() != this->get_culture() || other_population_unit->get_religion() != this->get_religion()) {
			continue;
		}

		if (!this->get_phenotype()->can_mix_with(other_population_unit->get_phenotype())) {
			continue;
		}

		this->mix_with(other_population_unit.get());
	}
}

std::vector<std::vector<std::string>> population_unit::get_tag_suffix_list_with_fallbacks() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	tag_list_with_fallbacks.push_back({this->get_culture()->get_identifier(), this->get_culture()->get_culture_group()->get_identifier()});
	tag_list_with_fallbacks.push_back({this->get_religion()->get_identifier(), this->get_religion()->get_religion_group()->get_identifier()});

	std::vector<std::string> phenotype_tag_with_fallbacks;
	phenotype_tag_with_fallbacks.push_back(this->get_phenotype()->get_identifier());
	for (metternich::phenotype *fallback_phenotype : this->get_phenotype()->get_icon_fallback_phenotypes()) {
		phenotype_tag_with_fallbacks.push_back(fallback_phenotype->get_identifier());
	}
	tag_list_with_fallbacks.push_back(std::move(phenotype_tag_with_fallbacks));

	return tag_list_with_fallbacks;
}

/**
**	@brief	Set the population unit's culture
**
**	@param	culture	The new culture
*/
void population_unit::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	this->culture = culture;
	emit culture_changed();

	if (this->get_phenotype() == nullptr) {
		metternich::phenotype *phenotype = culture->get_default_phenotype();
		this->set_phenotype(phenotype);
	}
}


/**
**	@brief	Mixing with another population unit
*/
void population_unit::mix_with(population_unit *other_population_unit)
{
	const int base_size = std::min(this->get_size(), other_population_unit->get_size());
	int size = base_size;
	size *= population_unit::mixing_factor_permyriad + this->get_holding()->get_population_growth(); //the mixing factor must be greater than the population growth, so that it has the potential to affect the demographic composition even for a growing population
	size /= 10000;
	if (size > 0) {
		size = random::generate(size);
	}
	size = std::max(size, 2); //2 instead of 1 so that it is always greater than pop. growth
	size = std::min(size, base_size);

	metternich::phenotype *mixed_phenotype = this->get_phenotype()->get_mixing_result(other_population_unit->get_phenotype());
	this->get_holding()->change_population_size(this->get_type(), this->get_culture(), this->get_religion(), mixed_phenotype, size * 2);

	this->change_size(-size);
	other_population_unit->change_size(-size);
}

/**
**	@brief	Set the population unit's size
**
**	@param	size	The size
*/
void population_unit::set_size(const int size)
{
	if (size == this->get_size()) {
		return;
	}

	const int old_size = this->get_size();
	this->size = std::max(size, 0);
	emit size_changed();

	const int size_change = this->get_size() - old_size;

	if (size_change > 0) {
		this->change_unemployed_size(size_change);
	} else {
		int total_employment_size_change = size_change;
		if (this->get_unemployed_size()) {
			const int unemployed_size_change = -std::min(this->get_unemployed_size(), abs(total_employment_size_change));
			this->change_unemployed_size(unemployed_size_change);
			total_employment_size_change -= unemployed_size_change;
		}

		if (total_employment_size_change != 0) {
			for (employment *employment : this->employments) {
				int employment_size = employment->get_employee_size(this);
				const int employment_size_change = -std::min(employment_size, abs(total_employment_size_change));
				employment->change_employee_size(this, employment_size_change);
				total_employment_size_change -= employment_size_change;

				if (total_employment_size_change == 0) {
					break;
				}
			}
		}
	}

	if (this->get_holding() != nullptr) {
		//change the population count for the population unit's holding
		this->get_holding()->change_population(size_change);
	}
}

/**
**	@brief	Get whether the population unit discounts any type
**
**	@return True if the population unit discounts any type, or false otherwise
*/
bool population_unit::discounts_any_type() const
{
	return this->get_discount_types().size() == population_type::get_all().size();
}

/**
**	@brief	Set whether the population unit discounts any type
**
**	@param	discount_any_type	True if the population unit should discount any type, or false otherwise
*/
void population_unit::set_discount_any_type(const bool discount_any_type)
{
	if (discount_any_type == this->discounts_any_type()) {
		return;
	}

	if (discount_any_type) {
		for (population_type *population_type : population_type::get_all()) {
			this->discount_types.insert(population_type);
		}
	} else {
		this->discount_types.clear();
	}
	emit discount_types_changed();
}

/**
**	@brief	Get the population unit's discount types as a QVariantList
**
**	@return The variant list
*/
QVariantList population_unit::get_discount_types_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_discount_types());
}

/**
**	@brief	Subtract the sizes of applicable existing population units from that of this one
*/
void population_unit::subtract_existing_sizes()
{
	if (this->get_holding() != nullptr) {
		this->subtract_existing_sizes_in_holding(this->get_holding());
	} else if (this->get_province() != nullptr) {
		this->subtract_existing_sizes_in_holdings(this->get_province()->get_settlement_holdings());
	} else if (this->get_region() != nullptr) {
		this->subtract_existing_sizes_in_holdings(this->get_region()->get_holdings());
	}
}

/**
**	@brief	Subtract the sizes of applicable existing population units in the holding from that of this one
**
**	@param	holding	The holding
*/
void population_unit::subtract_existing_sizes_in_holding(const metternich::holding *holding)
{
	for (const std::unique_ptr<population_unit> &population_unit : holding->get_population_units()) {
		if (&*population_unit == &*this) {
			continue;
		}

		if (this->get_discount_types().find(population_unit->get_type()) == this->get_discount_types().end()) {
			continue;
		}

		if (this->get_culture() != nullptr && population_unit->get_culture() != this->get_culture()) {
			continue;
		}

		if (this->get_religion() != nullptr && population_unit->get_religion() != this->get_religion()) {
			continue;
		}

		if (this->get_phenotype() != nullptr && population_unit->get_phenotype() != this->get_phenotype()) {
			continue;
		}

		this->change_size(-population_unit->get_size());
	}
}

/**
**	@brief	Subtract the sizes of applicable existing population units in the holdings from that of this one
**
**	@param	holdings	The holdings
*/
void population_unit::subtract_existing_sizes_in_holdings(const std::vector<metternich::holding *> &holdings)
{
	for (const metternich::holding *holding : holdings) {
		if (!holding->get_slot()->is_population_distribution_allowed()) {
			continue; //if cannot distribute population to a given holding, then don't subtract its population from the size either
		}

		this->subtract_existing_sizes_in_holding(holding);
	}
}

/**
**	@brief	Whether this population unit can be distributed to the given holding
**
**	@param	holding	The holding
**
**	@return	True if the population unit can be distributed to the holding, or false otherwise
*/
bool population_unit::can_distribute_to_holding(const metternich::holding *holding) const
{
	if (!holding->get_slot()->is_population_distribution_allowed()) {
		return false;
	}

	if (this->get_type()->get_holding_types().find(holding->get_type()) == this->get_type()->get_holding_types().end()) {
		return false;
	}

	if (this->discounts_existing()) {
		//the population unit can only be distributed to the given holding if there is no population unit there with the same type, culture, religion and phenotype as this one, if discount existing is enabled
		for (const std::unique_ptr<population_unit> &population_unit : holding->get_population_units()) {
			if (population_unit->get_type() != this->get_type()) {
				continue;
			}

			if (this->get_culture() != nullptr && population_unit->get_culture() != this->get_culture()) {
				continue;
			}

			if (this->get_religion() != nullptr && population_unit->get_religion() != this->get_religion()) {
				continue;
			}

			if (this->get_phenotype() != nullptr && population_unit->get_phenotype() != this->get_phenotype()) {
				continue;
			}

			return false;
		}
	}

	return true;
}

/**
**	@brief	Distribute the population unit to a number of holdings, in equal proportions
**
**	@param	holdings	The holdings
*/
void population_unit::distribute_to_holdings(const std::vector<metternich::holding *> &holdings)
{
	//set population for settlement holdings without population data
	int holding_count = 0; //count of settlement holdings for which the population will be applied

	for (const metternich::holding *holding : holdings) {
		if (this->can_distribute_to_holding(holding)) {
			holding_count++;
		}
	}

	if (holding_count == 0) {
		return;
	}

	//now, apply the remaining population to all settlement holdings without population set for them, in equal proportions
	const int size_per_holding = this->get_size() / holding_count;

	if (size_per_holding <= 0) {
		return;
	}

	for (metternich::holding *holding : holdings) {
		if (!this->can_distribute_to_holding(holding)) {
			continue;
		}

		auto population_unit = std::make_unique<metternich::population_unit>(this->get_type());
		population_unit->moveToThread(QApplication::instance()->thread());
		population_unit->set_holding(holding);
		population_unit->set_size(size_per_holding);
		if (this->get_culture() != nullptr) {
			population_unit->set_culture(this->get_culture());
		}
		if (this->get_religion() != nullptr) {
			population_unit->set_religion(this->get_religion());
		}
		if (this->get_phenotype() != nullptr) {
			population_unit->set_phenotype(this->get_phenotype());
		}
		holding->add_population_unit(std::move(population_unit));
	}
}

/**
**	@brief	Seek employment for the population unit
*/
void population_unit::seek_employment()
{
	for (auto &kv_pair : this->get_holding()->get_employments()) {
		employment *employment = kv_pair.second.get();

		if (employment->can_employ_population_unit(this)) {
			const int unused_employment_capacity = employment->get_unused_workforce_capacity();
			const int employee_size_change = std::min(this->get_unemployed_size(), unused_employment_capacity);
			employment->change_employee_size(this, employee_size_change);

			if (this->get_unemployed_size() == 0) {
				break;
			}
		}
	}
}

/**
**	@brief	Get the path to the population unit's icon
**
**	@return	The path to the icon
*/
std::filesystem::path population_unit::get_icon_path() const
{
	std::string base_tag = this->get_type()->get_icon_tag();

	std::filesystem::path icon_path = database::get_tagged_image_path(database::get_population_icons_path(), base_tag, this->get_tag_suffix_list_with_fallbacks(), "_small.png");
	return icon_path;
}

}
