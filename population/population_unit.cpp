#include "population/population_unit.h"

#include "database/gsml_data.h"
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
			population_unit->LoadHistory(const_cast<gsml_data &>(data_entry));

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
		this->set_culture(this->get_holding()->GetCulture());
	}

	if (this->get_religion() == nullptr) {
		this->set_religion(this->get_holding()->GetReligion());
	}

	this->set_unemployed_size(this->get_size());
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
		int employment_size_change = size_change;
		if (this->get_unemployed_size()) {
			const int unemployed_size_change = -std::min(this->get_unemployed_size(), abs(employment_size_change));
			this->change_unemployed_size(unemployed_size_change);
			employment_size_change -= unemployed_size_change;
		}

		if (employment_size_change != 0) {
			//FIXME: should remove employment size
		}
	}

	if (this->get_holding() != nullptr) {
		//change the population count for the population unit's holding
		this->get_holding()->ChangePopulation(size_change);
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
		this->subtract_existing_sizes_in_holdings(this->get_province()->GetHoldings());
	} else if (this->get_region() != nullptr) {
		this->subtract_existing_sizes_in_holdings(this->get_region()->GetHoldings());
	}
}

/**
**	@brief	Subtract the sizes of applicable existing population units in the holding from that of this one
**
**	@param	holding	The holding
*/
void population_unit::subtract_existing_sizes_in_holding(const metternich::Holding *holding)
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
void population_unit::subtract_existing_sizes_in_holdings(const std::vector<metternich::Holding *> &holdings)
{
	for (const metternich::Holding *holding : holdings) {
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
bool population_unit::can_distribute_to_holding(const metternich::Holding *holding) const
{
	if (this->get_type()->GetHoldingTypes().find(holding->GetType()) == this->get_type()->GetHoldingTypes().end()) {
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
void population_unit::distribute_to_holdings(const std::vector<metternich::Holding *> &holdings)
{
	//set population for settlement holdings without population data
	int holding_count = 0; //count of settlement holdings for which the population will be applied

	for (const metternich::Holding *holding : holdings) {
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

	for (metternich::Holding *holding : holdings) {
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
**	@brief	Set the population unit's employment size for a given employment typ
**
**	@param	employment_type	The employment type
**	@param	size			The new employment size
*/
void population_unit::set_employment_size(const EmploymentType *employment_type, const int size)
{
	if (size == this->get_employment_size(employment_type)) {
		return;
	}

	if (size < 0) {
		throw std::runtime_error("Tried to set a negative employment size for employment type \"" + employment_type->GetIdentifier() + "\" for a population unit.");
	}

	if (size == 0) {
		this->employment_sizes.erase(employment_type);
	} else {
		this->employment_sizes[employment_type] = size;
	}
}

}
