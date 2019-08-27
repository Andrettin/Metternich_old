#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QVariant>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Metternich {

class CSVData;
class Culture;
class Holding;
class HoldingType;
class LandedTitle;
class PopulationUnit;
class Region;
class Religion;
class Terrain;

class Province : public DataEntry, public DataType<Province>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::LandedTitle* county READ GetCounty WRITE SetCounty NOTIFY CountyChanged)
	Q_PROPERTY(Metternich::LandedTitle* duchy READ GetDuchy NOTIFY DuchyChanged)
	Q_PROPERTY(Metternich::LandedTitle* kingdom READ GetKingdom NOTIFY KingdomChanged)
	Q_PROPERTY(Metternich::LandedTitle* empire READ GetEmpire NOTIFY EmpireChanged)
	Q_PROPERTY(QColor color READ GetColor CONSTANT)
	Q_PROPERTY(QRect rect READ GetRect CONSTANT)
	Q_PROPERTY(QImage image READ GetImage NOTIFY ImageChanged)
	Q_PROPERTY(Metternich::Terrain* terrain READ GetTerrain WRITE SetTerrain NOTIFY TerrainChanged)
	Q_PROPERTY(Metternich::Culture* culture MEMBER Culture READ GetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion MEMBER Religion READ GetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)
	Q_PROPERTY(QVariantList holdings READ GetHoldingsQVariantList NOTIFY HoldingsChanged)
	Q_PROPERTY(Metternich::Holding* capital_holding READ GetCapitalHolding WRITE SetCapitalHolding NOTIFY CapitalHoldingChanged)
	Q_PROPERTY(bool selected READ IsSelected WRITE SetSelected NOTIFY SelectedChanged)
	Q_PROPERTY(bool selectable READ IsSelectable CONSTANT)

public:
	static constexpr const char *ClassIdentifier = "province";
	static constexpr const char *DatabaseFolder = "provinces";
	static constexpr const char *Prefix = "p_";

	static Province *Add(const std::string &identifier);
	static Province *GetByRGB(const QRgb &rgb, const bool should_find = true);

	static Province *GetSelectedProvince()
	{
		return Province::SelectedProvince;
	}

private:
	static inline std::map<QRgb, Province *> InstancesByRGB;
	static inline Province *SelectedProvince = nullptr;

public:
	Province(const std::string &identifier);
	virtual ~Province() override;

	virtual void ProcessGSMLProperty(const GSMLProperty &property) override;
	virtual void ProcessGSMLScope(const GSMLData &scope) override;
	virtual void ProcessGSMLDatedScope(const GSMLData &scope, const QDateTime &date) override;
	virtual void InitializeHistory() override;
	virtual void Check() const override;

	void DoDay();
	void DoMonth();

	virtual std::string GetName() const override;

	LandedTitle *GetCounty() const
	{
		return this->County;
	}

	void SetCounty(LandedTitle *county);

	LandedTitle *GetDuchy() const;
	LandedTitle *GetKingdom() const;
	LandedTitle *GetEmpire() const;

	const QColor &GetColor() const
	{
		return this->Color;
	}

	const QRect &GetRect() const
	{
		return this->Rect;
	}

	void CreateImage(const std::vector<int> &pixel_indexes);
	void SetBorderPixels(const std::vector<int> &pixel_indexes);
	void UpdateImage();

	const QImage &GetImage() const
	{
		return this->Image;
	}

	Metternich::Terrain *GetTerrain() const
	{
		return this->Terrain;
	}

	void SetTerrain(Terrain *terrain);

	Metternich::Culture *GetCulture() const
	{
		return this->Culture;
	}

	Metternich::Religion *GetReligion() const
	{
		return this->Religion;
	}

	int GetPopulation() const
	{
		return this->Population;
	}

	void SetPopulation(const int population);

	void ChangePopulation(const int change)
	{
		this->SetPopulation(this->GetPopulation() + change);
	}

	void CalculatePopulation();

	int GetPopulationCapacityAdditiveModifier() const
	{
		return this->PopulationCapacityAdditiveModifier;
	}

	void SetPopulationCapacityAdditiveModifier(const int population_capacity_modifier);

	void ChangePopulationCapacityAdditiveModifier(const int change)
	{
		this->SetPopulationCapacityAdditiveModifier(this->GetPopulationCapacityAdditiveModifier() + change);
	}

	int GetPopulationCapacityModifier() const
	{
		return this->PopulationCapacityModifier;
	}

	void SetPopulationCapacityModifier(const int population_capacity_modifier);

	void ChangePopulationCapacityModifier(const int change)
	{
		this->SetPopulationCapacityModifier(this->GetPopulationCapacityModifier() + change);
	}

	int GetPopulationGrowthModifier() const
	{
		return this->PopulationGrowthModifier;
	}

	void SetPopulationGrowthModifier(const int population_capacity_modifier);

	void ChangePopulationGrowthModifier(const int change)
	{
		this->SetPopulationGrowthModifier(this->GetPopulationGrowthModifier() + change);
	}

	const std::vector<Holding *> &GetHoldings() const
	{
		return this->Holdings;
	}

	QVariantList GetHoldingsQVariantList() const;
	Holding *GetHolding(LandedTitle *barony) const;
	void CreateHolding(LandedTitle *barony, HoldingType *type);
	void DestroyHolding(LandedTitle *barony);

	Holding *GetCapitalHolding() const
	{
		return this->CapitalHolding;
	}

	void SetCapitalHolding(Holding *holding)
	{
		if (holding == this->GetCapitalHolding()) {
			return;
		}

		this->CapitalHolding = holding;
		emit CapitalHoldingChanged();
	}

	const std::vector<Region *> &GetRegions() const
	{
		return this->Regions;
	}

	void AddRegion(Region *region)
	{
		this->Regions.push_back(region);
	}

	void RemoveRegion(Region *region)
	{
		this->Regions.erase(std::remove(this->Regions.begin(), this->Regions.end(), region), this->Regions.end());
	}

	void AddBorderProvince(Province *province)
	{
		this->BorderProvinces.insert(province);
	}

	bool BordersWater() const;
	bool BordersRiver() const;
	bool IsCoastal() const;

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify_engine_interface = true);
	bool IsSelectable() const;

	const std::vector<std::unique_ptr<PopulationUnit>> &GetPopulationUnits() const
	{
		return this->PopulationUnits;
	}

	void AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit);

signals:
	void CountyChanged();
	void DuchyChanged();
	void KingdomChanged();
	void EmpireChanged();
	void ImageChanged();
	void TerrainChanged();
	void CultureChanged();
	void ReligionChanged();
	void PopulationChanged();
	void HoldingsChanged();
	void CapitalHoldingChanged();
	void SelectedChanged();

private:
	LandedTitle *County = nullptr;
	QColor Color; //the color used to identify the province in the province map
	QRect Rect; //the rectangle that the province occupies
	QImage Image; //the province's image to be drawn on-screen
	Metternich::Terrain *Terrain = nullptr;
	Metternich::Culture *Culture = nullptr;
	Metternich::Religion *Religion = nullptr;
	int Population = 0; //the sum of the population of all of the province's settlement holdings
	int PopulationCapacityAdditiveModifier = 0; //the population capacity additive modifier which the province provides to its holdings
	int PopulationCapacityModifier = 0; //the population capacity modifier which the province provides to its holdings
	int PopulationGrowthModifier = 0; //the population growth modifier which the province provides to its holdings
	std::vector<Holding *> Holdings;
	std::map<LandedTitle *, std::unique_ptr<Holding>> HoldingsByBarony; //the province's holdings, mapped to their respective baronies
	Holding *CapitalHolding = nullptr;
	std::vector<Region *> Regions; //the regions to which this province belongs
	std::set<Province *> BorderProvinces; //provinces bordering this one
	bool Selected = false;
	std::vector<std::unique_ptr<PopulationUnit>> PopulationUnits; //population units set for this province in history, used during initialization to generate population units in the province's settlements
};

}
