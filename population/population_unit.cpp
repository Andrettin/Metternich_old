#include "population/population_unit.h"

#include "database/gsml_data.h"
#include "economy/employment.h"
#include "economy/employment_type.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/province.h"
#include "map/region.h"
#include "population/population_type.h"
#include "util.h"

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
			PopulationType *type = PopulationType::Get(type_identifier);
			auto population_unit(std::make_unique<population_unit>(type));
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

	this->set_unemployed_size(this->get_size());
}

/**
**	@brief	Do the population unit's monthly actions
*/
void population_unit::do_month()
{
	if (this->get_unemployed_size() > 0) {
		this->seek_employment();
	}
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
		this->subtract_existing_sizes_in_holdings(this->get_province()->get_holdings());
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

		if (!this->discounts_any_type() && this->get_discount_types().find(population_unit->get_type()) == this->get_discount_types().end()) {
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
	if (this->get_type()->get_holding_types().find(holding->get_type()) == this->get_type()->get_holding_types().end()) {
		return false;
	}

	if (this->discounts_existing()) {
		//the population unit can only be distributed to the given holding if there is no population unit there with the same type as this one, if discount existing is enabled
		for (const std::unique_ptr<population_unit> &population_unit : holding->get_population_units()) {
			if (this->get_type() == population_unit->get_type()) {
				return false;
			}
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

		auto population_unit(std::make_unique<population_unit>(this->get_type()));
		population_unit->moveToThread(QApplication::instance()->thread());
		population_unit->set_holding(holding);
		population_unit->set_size(size_per_holding);
		if (this->get_culture() != nullptr) {
			population_unit->set_culture(this->get_culture());
		}
		if (this->get_religion() != nullptr) {
			population_unit->set_religion(this->get_religion());
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

}
