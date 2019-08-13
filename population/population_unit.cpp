#include "population/population_unit.h"

#include "database/gsml_data.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/province.h"
#include "map/region.h"
#include "population/population_type.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Process the history database for population units
*/
void PopulationUnit::ProcessHistoryDatabase()
{
	//simple data types are only loaded in history, instanced directly based on their GSML data
	for (const GSMLData &gsml_data : PopulationUnit::GSMLHistoryDataToProcess) {
		for (const GSMLData &data_entry : gsml_data.GetChildren()) {
			const std::string type_identifier = data_entry.GetTag();
			PopulationType *type = PopulationType::Get(type_identifier);
			auto population_unit = std::make_unique<PopulationUnit>(type);
			population_unit->LoadHistory(const_cast<GSMLData &>(data_entry));

			if (population_unit->GetHolding() != nullptr) {
				population_unit->GetHolding()->AddPopulationUnit(std::move(population_unit));
			} else if (population_unit->GetProvince() != nullptr) {
				population_unit->GetProvince()->AddPopulationUnit(std::move(population_unit));
			} else if (population_unit->GetRegion() != nullptr) {
				population_unit->GetRegion()->AddPopulationUnit(std::move(population_unit));
			} else {
				throw std::runtime_error("Population unit of type \"" + type_identifier + "\" belongs to neither a holding, nor a province, nor a region.");
			}
		}
	}

	PopulationUnit::GSMLHistoryDataToProcess.clear();
}

/**
**	@brief	Set the population unit's size
**
**	@param	size	The size
*/
void PopulationUnit::SetSize(const int size)
{
	if (size == this->GetSize()) {
		return;
	}

	this->Size = std::max(size, 0);
	emit SizeChanged();
}

/**
**	@brief	Get the population unit's subtraction types as a QVariantList
**
**	@return The variant list
*/
QVariantList PopulationUnit::GetSubtractionTypesQVariantList() const
{
	return ContainerToQVariantList(this->GetSubtractionTypes());
}

/**
**	@brief	Subtract the sizes of applicable existing population units from that of this one
*/
void PopulationUnit::SubtractExistingSizes()
{
	if (this->GetHolding() != nullptr) {
		this->SubtractExistingSizesInHolding(this->GetHolding());
	} else if (this->GetProvince() != nullptr) {
		this->SubtractExistingSizesInHoldings(this->GetProvince()->GetHoldings());
	} else if (this->GetRegion() != nullptr) {
		this->SubtractExistingSizesInHoldings(this->GetRegion()->GetHoldings());
	}
}

/**
**	@brief	Subtract the sizes of applicable existing population units in the holding from that of this one
**
**	@param	holding	The holding
*/
void PopulationUnit::SubtractExistingSizesInHolding(const Metternich::Holding *holding)
{
	for (const std::unique_ptr<PopulationUnit> &population_unit : holding->GetPopulationUnits()) {
		if (&*population_unit == &*this) {
			continue;
		}

		if (!this->GetSubtractAnyType() && this->SubtractionTypes.find(population_unit->GetType()) == this->SubtractionTypes.end()) {
			continue;
		}

		this->ChangeSize(-population_unit->GetSize());
	}
}

/**
**	@brief	Subtract the sizes of applicable existing population units in the holdings from that of this one
**
**	@param	holdings	The holdings
*/
void PopulationUnit::SubtractExistingSizesInHoldings(const std::vector<Metternich::Holding *> &holdings)
{
	for (const Metternich::Holding *holding : holdings) {
		this->SubtractExistingSizesInHolding(holding);
	}
}

/**
**	@brief	Whether this population unit can be distributed to the given holding
**
**	@param	holding	The holding
**
**	@return	True if the population unit can be distributed to the holding, or false otherwise
*/
bool PopulationUnit::CanDistributeToHolding(const Metternich::Holding *holding) const
{
	//the population unit can only be distributed to the given holding if there is no population unit there with the same type as this one
	for (const std::unique_ptr<PopulationUnit> &population_unit : holding->GetPopulationUnits()) {
		if (this->GetType() == population_unit->GetType()) {
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
void PopulationUnit::DistributeToHoldings(const std::vector<Metternich::Holding *> &holdings)
{
	//set population for settlement holdings without population data
	int holding_count = 0; //count of settlement holdings for which the population will be applied

	for (const Metternich::Holding *holding : holdings) {
		if (this->CanDistributeToHolding(holding)) {
			holding_count++;
		}
	}

	if (holding_count == 0) {
		return;
	}

	//now, apply the remaining population to all settlement holdings without population set for them, in equal proportions
	const int size_per_holding = this->GetSize() / holding_count;

	if (size_per_holding <= 0) {
		return;
	}

	for (Metternich::Holding *holding : holdings) {
		if (!this->CanDistributeToHolding(holding)) {
			continue;
		}

		auto population_unit = std::make_unique<PopulationUnit>(this->GetType());
		population_unit->SetSize(size_per_holding);
		if (this->GetCulture() != nullptr) {
			population_unit->SetCulture(this->GetCulture());
		}
		if (this->GetReligion() != nullptr) {
			population_unit->SetReligion(this->GetReligion());
		}
		holding->AddPopulationUnit(std::move(population_unit));
	}
}

}
