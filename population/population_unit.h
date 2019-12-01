#pragma once

#include "database/data_entry.h"
#include "database/simple_data_type.h"

#include <QObject>

#include <set>

namespace metternich {

class culture;
class employment;
class holding;
class phenotype;
class population_type;
class province;
class region;
class religion;

class population_unit : public data_entry_base, public simple_data_type<population_unit>
{
	Q_OBJECT

	Q_PROPERTY(metternich::population_type* type MEMBER type READ get_type NOTIFY type_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(metternich::phenotype* phenotype READ get_phenotype WRITE set_phenotype NOTIFY phenotype_changed)
	Q_PROPERTY(int size READ get_size WRITE set_size NOTIFY size_changed)
	Q_PROPERTY(metternich::holding* holding READ get_holding WRITE set_holding NOTIFY holding_changed)
	Q_PROPERTY(metternich::province* province READ get_province WRITE set_province NOTIFY province_changed)
	Q_PROPERTY(metternich::region* region READ get_region WRITE set_region NOTIFY region_changed)
	Q_PROPERTY(int wealth READ get_wealth NOTIFY wealth_changed)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring NOTIFY icon_path_changed)
	Q_PROPERTY(bool discount_existing READ discounts_existing WRITE set_discount_existing NOTIFY discount_existing_changed)
	Q_PROPERTY(bool discount_any_type READ discounts_any_type WRITE set_discount_any_type NOTIFY discount_types_changed)
	Q_PROPERTY(QVariantList discount_types READ get_discount_types_qvariant_list NOTIFY discount_types_changed)

public:
	static constexpr const char *database_folder = "population_units";
	static constexpr int mixing_factor_permyriad = 1; //up to this fraction of people will "mix" per month per mixing check

	static void process_history_database();

	population_unit(population_type *type) : type(type)
	{
		connect(this, &population_unit::type_changed, this, &population_unit::icon_path_changed);
		connect(this, &population_unit::culture_changed, this, &population_unit::icon_path_changed);
		connect(this, &population_unit::religion_changed, this, &population_unit::icon_path_changed);
		connect(this, &population_unit::phenotype_changed, this, &population_unit::icon_path_changed);
	}

	virtual ~population_unit() override
	{
	}

	virtual void initialize_history() override;

	void do_month();
	void do_mixing();

	std::vector<std::vector<std::string>> get_tag_suffix_list_with_fallbacks() const;

	population_type *get_type() const
	{
		return this->type;
	}

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture);

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

	metternich::phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	void set_phenotype(phenotype *phenotype)
	{
		if (phenotype == this->get_phenotype()) {
			return;
		}

		this->phenotype = phenotype;
		emit phenotype_changed();
	}

	void mix_with(population_unit *other_population_unit);

	int get_size() const
	{
		return this->size;
	}

	void set_size(const int size);

	void change_size(const int change)
	{
		this->set_size(this->get_size() + change);
	}

	metternich::holding *get_holding() const
	{
		return this->holding;
	}

	void set_holding(holding *holding)
	{
		if (holding == this->get_holding()) {
			return;
		}

		this->holding = holding;
		emit holding_changed();
	}

	metternich::province *get_province() const
	{
		return this->province;
	}

	void set_province(province *province)
	{
		if (province == this->get_province()) {
			return;
		}

		this->province = province;
		emit province_changed();
	}

	metternich::region *get_region() const
	{
		return this->region;
	}

	void set_region(region *region)
	{
		if (region == this->get_region()) {
			return;
		}

		this->region = region;
		emit region_changed();
	}

	bool discounts_existing() const
	{
		return this->discount_existing;
	}

	void set_discount_existing(const bool discount_existing)
	{
		if (discount_existing == this->discounts_existing()) {
			return;
		}

		this->discount_existing = discount_existing;
		if (discount_existing) {
			this->discount_types.insert(this->get_type()); //this population unit's type is implicitly added to the discount types if discount_existing is set to true
		} else {
			//if is being set to false, set the discount_any_type to false as well, and clear the discount types, as both are no longer applicable
			this->discount_types.clear();
		}
		emit discount_existing_changed();
	}

	bool discounts_any_type() const;
	void set_discount_any_type(const bool discount_any_type);

	const std::set<population_type *> &get_discount_types() const
	{
		return this->discount_types;
	}

	QVariantList get_discount_types_qvariant_list() const;

	Q_INVOKABLE void add_discount_type(population_type *type)
	{
		this->discount_types.insert(type);
		emit discount_types_changed();
	}

	Q_INVOKABLE void remove_discount_type(population_type *type)
	{
		this->discount_types.erase(type);
		emit discount_types_changed();
	}

	void subtract_existing_sizes();
	void subtract_existing_sizes_in_holding(const holding *holding);
	void subtract_existing_sizes_in_holdings(const std::vector<holding *> &holdings);
	bool can_distribute_to_holding(const holding *holding) const;
	void distribute_to_holdings(const std::vector<holding *> &holdings);

	int get_unemployed_size() const
	{
		return this->unemployed_size;
	}

	void set_unemployed_size(const int size)
	{
		if (size == this->get_unemployed_size()) {
			return;
		}

		this->unemployed_size = size;
	}

	void change_unemployed_size(const int change)
	{
		this->set_unemployed_size(this->get_unemployed_size() + change);
	}

	void add_employment(employment *employment)
	{
		this->employments.insert(employment);
	}

	void remove_employment(employment *employment)
	{
		this->employments.erase(employment);
	}

	void seek_employment();

	int get_wealth() const
	{
		return this->wealth;
	}

	void set_wealth(const int wealth)
	{
		if (wealth == this->get_wealth()) {
			return;
		}

		this->wealth = wealth;
		emit wealth_changed();
	}

	void change_wealth(const int change)
	{
		this->set_wealth(this->get_wealth() + change);
	}

	const std::filesystem::path &get_icon_path() const;

	QString get_icon_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_icon_path().string());
	}

signals:
	void type_changed();
	void culture_changed();
	void religion_changed();
	void phenotype_changed();
	void size_changed();
	void holding_changed();
	void province_changed();
	void region_changed();
	void discount_existing_changed();
	void discount_types_changed();
	void wealth_changed();
	void icon_path_changed();

private:
	population_type *type = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	metternich::phenotype *phenotype = nullptr;
	int size = 0; //the size of the population unit, in number of individuals
	metternich::holding *holding = nullptr; //the settlement holding where this population unit lives
	metternich::province *province = nullptr; //the province where this population unit lives; used only during initialization to generate population units in settlements in the province
	metternich::region *region = nullptr; //the region where this population unit lives; used only during initialization to generate population units in settlements in the region
	bool discount_existing = false; //whether to discount the size of existing population units (in this population unit's holding, province or region) of the types given in DiscountTypes from that of this one
	bool discount_any_type = false; //whether to discount the size of any existing population units from that of this one
	std::set<population_type *> discount_types; //the sizes of population units belonging to these types will be discounted from that of this population unit
	std::set<employment *> employments;
	int unemployed_size = 0; //the amount of people from this population unit which are unemployed
	int wealth = 0; //the wealth belonging to the population unit
};

}
