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

namespace metternich {

class CSVData;
class culture;
class holding;
class holding_type;
class LandedTitle;
class PopulationType;
class population_unit;
class region;
class religion;
class Terrain;

class Province : public data_entry, public data_type<Province>
{
	Q_OBJECT

	Q_PROPERTY(metternich::LandedTitle* county READ get_county WRITE SetCounty NOTIFY CountyChanged)
	Q_PROPERTY(metternich::LandedTitle* duchy READ GetDuchy NOTIFY DuchyChanged)
	Q_PROPERTY(metternich::LandedTitle* kingdom READ GetKingdom NOTIFY KingdomChanged)
	Q_PROPERTY(metternich::LandedTitle* empire READ GetEmpire NOTIFY EmpireChanged)
	Q_PROPERTY(QColor color READ GetColor CONSTANT)
	Q_PROPERTY(QRect rect READ GetRect CONSTANT)
	Q_PROPERTY(QImage image READ GetImage NOTIFY ImageChanged)
	Q_PROPERTY(metternich::Terrain* terrain READ GetTerrain WRITE SetTerrain NOTIFY TerrainChanged)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)
	Q_PROPERTY(QVariantList holdings READ get_holdings_qvariant_list NOTIFY holdings_changed)
	Q_PROPERTY(metternich::holding* capital_holding READ get_capital_holding WRITE set_capital_holding NOTIFY capital_holding_changed)
	Q_PROPERTY(bool selected READ IsSelected WRITE SetSelected NOTIFY SelectedChanged)
	Q_PROPERTY(bool selectable READ IsSelectable CONSTANT)

public:
	static constexpr const char *class_identifier = "province";
	static constexpr const char *database_folder = "provinces";
	static constexpr const char *Prefix = "p_";

	static Province *add(const std::string &identifier);
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

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date) override;
	virtual void initialize_history() override;
	virtual void check() const override;

	void DoDay();
	void DoMonth();

	virtual std::string get_name() const override;

	LandedTitle *get_county() const
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

	metternich::Terrain *GetTerrain() const
	{
		return this->Terrain;
	}

	void SetTerrain(Terrain *terrain);

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture)
	{
		if (culture == this->get_culture()) {
			return;
		}

		this->culture = culture;
		emit culture_changed();
	}

	metternich::religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(religion *religion)
	{
		if (religion == this->get_religion()) {
			return;
		}

		this->religion = religion;
		emit religion_changed();
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

	void CalculatePopulationGroups();

	const std::vector<holding *> &get_holdings() const
	{
		return this->holdings;
	}

	QVariantList get_holdings_qvariant_list() const;
	holding *get_holding(LandedTitle *barony) const;
	void create_holding(LandedTitle *barony, holding_type *type);
	void destroy_holding(LandedTitle *barony);

	holding *get_capital_holding() const
	{
		return this->capital_holding;
	}

	void set_capital_holding(holding *holding)
	{
		if (holding == this->get_capital_holding()) {
			return;
		}

		this->capital_holding = holding;
		emit capital_holding_changed();
	}

	const std::vector<region *> &get_regions() const
	{
		return this->regions;
	}

	void add_region(region *region)
	{
		this->regions.push_back(region);
	}

	void remove_region(region *region)
	{
		this->regions.erase(std::remove(this->regions.begin(), this->regions.end(), region), this->regions.end());
	}

	void add_border_province(Province *province)
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

	const std::vector<std::unique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(std::unique_ptr<population_unit> &&population_unit);

	Q_INVOKABLE QVariantList get_population_per_type_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_culture_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_religion_qvariant_list() const;

signals:
	void CountyChanged();
	void DuchyChanged();
	void KingdomChanged();
	void EmpireChanged();
	void ImageChanged();
	void TerrainChanged();
	void culture_changed();
	void religion_changed();
	void PopulationChanged();
	void populationGroupsChanged();
	void holdings_changed();
	void capital_holding_changed();
	void SelectedChanged();

private:
	LandedTitle *County = nullptr;
	QColor Color; //the color used to identify the province in the province map
	QRect Rect; //the rectangle that the province occupies
	QImage Image; //the province's image to be drawn on-screen
	metternich::Terrain *Terrain = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	int Population = 0; //the sum of the population of all of the province's settlement holdings
	int PopulationCapacityAdditiveModifier = 0; //the population capacity additive modifier which the province provides to its holdings
	int PopulationCapacityModifier = 0; //the population capacity modifier which the province provides to its holdings
	int PopulationGrowthModifier = 0; //the population growth modifier which the province provides to its holdings
	std::vector<holding *> holdings;
	std::map<LandedTitle *, std::unique_ptr<holding>> holdings_by_barony; //the province's holdings, mapped to their respective baronies
	holding *capital_holding = nullptr;
	std::vector<region *> regions; //the regions to which this province belongs
	std::set<Province *> BorderProvinces; //provinces bordering this one
	bool Selected = false;
	std::vector<std::unique_ptr<population_unit>> population_units; //population units set for this province in history, used during initialization to generate population units in the province's settlements
	std::map<PopulationType *, int> PopulationPerType; //the population for each population type
	std::map<metternich::culture *, int> population_per_culture; //the population for each culture
	std::map<metternich::religion *, int> population_per_religion; //the population for each religion
};

}
