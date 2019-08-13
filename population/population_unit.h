#pragma once

#include "database/data_entry.h"
#include "database/data_type_base.h"

#include <QObject>

#include <set>

namespace Metternich {

class Culture;
class Holding;
class PopulationType;
class Province;
class Region;
class Religion;

class PopulationUnit : public DataEntryBase, public SimpleDataType<PopulationUnit>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::PopulationType* type MEMBER Type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::Culture* culture READ GetCulture WRITE SetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion READ GetReligion WRITE SetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(int size READ GetSize WRITE SetSize NOTIFY SizeChanged)
	Q_PROPERTY(Metternich::Holding* holding READ GetHolding WRITE SetHolding NOTIFY HoldingChanged)
	Q_PROPERTY(Metternich::Province* province READ GetProvince WRITE SetProvince NOTIFY ProvinceChanged)
	Q_PROPERTY(Metternich::Region* region READ GetRegion WRITE SetRegion NOTIFY RegionChanged)
	Q_PROPERTY(bool subtract_existing READ GetSubtractExisting WRITE SetSubtractExisting NOTIFY SubtractExistingChanged)
	Q_PROPERTY(bool subtract_any_type READ GetSubtractAnyType WRITE SetSubtractAnyType NOTIFY SubtractAnyTypeChanged)
	Q_PROPERTY(QVariantList subtraction_types READ GetSubtractionTypesQVariantList)

public:
	static constexpr const char *DatabaseFolder = "population_units";

	static void ProcessHistoryDatabase();

	PopulationUnit(PopulationType *type) : Type(type) {}

	PopulationType *GetType() const
	{
		return this->Type;
	}

	Metternich::Culture *GetCulture() const
	{
		return this->Culture;
	}

	void SetCulture(Culture *culture)
	{
		if (culture == this->GetCulture()) {
			return;
		}

		this->Culture = culture;
		emit CultureChanged();
	}

	Metternich::Religion *GetReligion() const
	{
		return this->Religion;
	}

	void SetReligion(Religion *religion)
	{
		if (religion == this->GetReligion()) {
			return;
		}

		this->Religion = religion;
		emit ReligionChanged();
	}

	int GetSize() const
	{
		return this->Size;
	}

	void SetSize(const int size);

	void ChangeSize(const int change)
	{
		this->SetSize(this->GetSize() + change);
	}

	Metternich::Holding *GetHolding() const
	{
		return this->Holding;
	}

	void SetHolding(Holding *holding)
	{
		if (holding == this->GetHolding()) {
			return;
		}

		this->Holding = holding;
		emit HoldingChanged();
	}

	Metternich::Province *GetProvince() const
	{
		return this->Province;
	}

	void SetProvince(Province *province)
	{
		if (province == this->GetProvince()) {
			return;
		}

		this->Province = province;
		emit ProvinceChanged();
	}

	Metternich::Region *GetRegion() const
	{
		return this->Region;
	}

	void SetRegion(Region *region)
	{
		if (region == this->GetRegion()) {
			return;
		}

		this->Region = region;
		emit RegionChanged();
	}

	bool GetSubtractExisting() const
	{
		return this->SubtractExisting;
	}

	void SetSubtractExisting(const bool subtract_existing)
	{
		if (subtract_existing == this->GetSubtractExisting()) {
			return;
		}

		this->SubtractExisting = subtract_existing;
		if (subtract_existing) {
			this->SubtractionTypes.insert(this->GetType()); //this population unit's type is implicitly added to the subtraction types is SubtractExisting is set to true
		} else {
			//if is being set to false, set the SubtractAnyType to false as well, and clear the subtraction types, as both are no longer applicable
			this->SetSubtractAnyType(false);
			this->SubtractionTypes.clear();
		}
		emit SubtractExistingChanged();
	}

	bool GetSubtractAnyType() const
	{
		return this->SubtractAnyType;
	}

	void SetSubtractAnyType(const bool subtract_any_type)
	{
		if (subtract_any_type == this->GetSubtractAnyType()) {
			return;
		}

		this->SubtractAnyType = subtract_any_type;
		emit SubtractAnyTypeChanged();
	}

	const std::set<PopulationType *> &GetSubtractionTypes() const
	{
		return this->SubtractionTypes;
	}

	QVariantList GetSubtractionTypesQVariantList() const;

	Q_INVOKABLE void AddSubtractionType(PopulationType *type)
	{
		this->SubtractionTypes.insert(type);
	}

	Q_INVOKABLE void RemoveSubtractionType(PopulationType *type)
	{
		this->SubtractionTypes.erase(type);
	}

	void SubtractExistingSizes();
	void SubtractExistingSizesInHolding(const Holding *holding);
	void SubtractExistingSizesInHoldings(const std::vector<Holding *> &holdings);
	bool CanDistributeToHolding(const Holding *holding) const;
	void DistributeToHoldings(const std::vector<Holding *> &holdings);

signals:
	void TypeChanged();
	void CultureChanged();
	void ReligionChanged();
	void SizeChanged();
	void HoldingChanged();
	void ProvinceChanged();
	void RegionChanged();
	void SubtractExistingChanged();
	void SubtractAnyTypeChanged();

private:
	PopulationType *Type = nullptr;
	Metternich::Culture *Culture = nullptr;
	Metternich::Religion *Religion = nullptr;
	int Size = 0; //the size of the population unit, in number of individuals
	Metternich::Holding *Holding = nullptr; //the settlement holding where this population unit lives
	Metternich::Province *Province = nullptr; //the province where this population unit lives; used only during initialization to generate population units in settlements in the province
	Metternich::Region *Region = nullptr; //the region where this population unit lives; used only during initialization to generate population units in settlements in the region
	bool SubtractExisting = false; //whether to subtract the size of existing population units (in this population unit's holding, province or region) of the types given in SubtractionTypes from that of this one
	bool SubtractAnyType = false; //whether to subtract the size of any existing population units from that of this one
	std::set<PopulationType *> SubtractionTypes; //the sizes of population units belonging to these types will be subtracted from that of this population unit
};

}
