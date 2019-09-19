#pragma once

#include "database/data_entry.h"
#include "database/simple_data_type.h"

#include <QObject>

#include <set>

namespace metternich {

class culture;
class employment;
class holding;
class PopulationType;
class Province;
class Region;
class Religion;

class population_unit : public DataEntryBase, public SimpleDataType<population_unit>
{
	Q_OBJECT

	Q_PROPERTY(metternich::PopulationType* type MEMBER type READ get_type NOTIFY type_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::Religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(int size READ get_size WRITE set_size NOTIFY size_changed)
	Q_PROPERTY(metternich::holding* holding READ get_holding WRITE set_holding NOTIFY holding_changed)
	Q_PROPERTY(metternich::Province* province READ get_province WRITE set_province NOTIFY province_changed)
	Q_PROPERTY(metternich::Region* region READ get_region WRITE set_region NOTIFY region_changed)
	Q_PROPERTY(int wealth READ get_wealth NOTIFY wealth_changed)
	Q_PROPERTY(bool discount_existing READ discounts_existing WRITE set_discount_existing NOTIFY discount_existing_changed)
	Q_PROPERTY(bool discount_any_type READ discounts_any_type WRITE set_discount_any_type NOTIFY discount_any_type_changed)
	Q_PROPERTY(QVariantList discount_types READ get_discount_types_qvariant_list)

public:
	static constexpr const char *DatabaseFolder = "population_units";

	static void process_history_database();

	population_unit(PopulationType *type) : type(type) {}

	virtual void initialize_history() override;

	void do_month();

	PopulationType *get_type() const
	{
		return this->type;
	}

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

	metternich::Religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(Religion *religion)
	{
		if (religion == this->get_religion()) {
			return;
		}

		this->religion = religion;
		emit religion_changed();
	}

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

	metternich::Province *get_province() const
	{
		return this->province;
	}

	void set_province(Province *province)
	{
		if (province == this->get_province()) {
			return;
		}

		this->province = province;
		emit province_changed();
	}

	metternich::Region *get_region() const
	{
		return this->region;
	}

	void set_region(Region *region)
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
			this->discount_types.insert(this->get_type()); //this population unit's type is implicitly added to the discount types if DiscountExisting is set to true
		} else {
			//if is being set to false, set the DiscountAnyType to false as well, and clear the discount types, as both are no longer applicable
			this->set_discount_any_type(false);
			this->discount_types.clear();
		}
		emit discount_existing_changed();
	}

	bool discounts_any_type() const
	{
		return this->discount_any_type;
	}

	void set_discount_any_type(const bool discount_any_type)
	{
		if (discount_any_type == this->discounts_any_type()) {
			return;
		}

		this->discount_any_type = discount_any_type;
		emit discount_any_type_changed();
	}

	const std::set<PopulationType *> &get_discount_types() const
	{
		return this->discount_types;
	}

	QVariantList get_discount_types_qvariant_list() const;

	Q_INVOKABLE void add_discount_type(PopulationType *type)
	{
		this->discount_types.insert(type);
	}

	Q_INVOKABLE void remove_discount_type(PopulationType *type)
	{
		this->discount_types.erase(type);
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

signals:
	void type_changed();
	void culture_changed();
	void religion_changed();
	void size_changed();
	void holding_changed();
	void province_changed();
	void region_changed();
	void discount_existing_changed();
	void discount_any_type_changed();
	void wealth_changed();

private:
	PopulationType *type = nullptr;
	metternich::culture *culture = nullptr;
	metternich::Religion *religion = nullptr;
	int size = 0; //the size of the population unit, in number of individuals
	metternich::holding *holding = nullptr; //the settlement holding where this population unit lives
	metternich::Province *province = nullptr; //the province where this population unit lives; used only during initialization to generate population units in settlements in the province
	metternich::Region *region = nullptr; //the region where this population unit lives; used only during initialization to generate population units in settlements in the region
	bool discount_existing = false; //whether to discount the size of existing population units (in this population unit's holding, province or region) of the types given in DiscountTypes from that of this one
	bool discount_any_type = false; //whether to discount the size of any existing population units from that of this one
	std::set<PopulationType *> discount_types; //the sizes of population units belonging to these types will be discounted from that of this population unit
	std::set<employment *> employments;
	int unemployed_size = 0; //the amount of people from this population unit which are unemployed
	int wealth = 0; //the wealth belonging to the population unit
};

}
