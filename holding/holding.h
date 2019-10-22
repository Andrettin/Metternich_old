#pragma once

#include "database/data_entry.h"

#include <QVariant>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace metternich {

class Building;
class Character;
class commodity;
class culture;
class employment;
class employment_type;
class holding_type;
class IdentifiableModifier;
class landed_title;
class PopulationType;
class population_unit;
class province;
class religion;

class holding : public data_entry
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring NOTIFY name_changed)
	Q_PROPERTY(QString titled_name READ get_titled_name_qstring NOTIFY titled_name_changed)
	Q_PROPERTY(metternich::holding_type* type READ get_type NOTIFY type_changed)
	Q_PROPERTY(metternich::landed_title* barony READ get_barony CONSTANT)
	Q_PROPERTY(int population READ get_population WRITE set_population NOTIFY population_changed)
	Q_PROPERTY(int population_capacity READ get_population_capacity NOTIFY population_capacity_changed)
	Q_PROPERTY(int population_growth READ get_population_growth NOTIFY population_growth_changed)
	Q_PROPERTY(QVariantList population_units READ get_population_units_qvariant_list NOTIFY population_units_changed)
	Q_PROPERTY(QVariantList buildings READ get_buildings_qvariant_list NOTIFY buildings_changed)
	Q_PROPERTY(QVariantList available_buildings READ get_available_buildings_qvariant_list NOTIFY available_buildings_changed)
	Q_PROPERTY(metternich::Building* under_construction_building READ get_under_construction_building NOTIFY under_construction_building_changed)
	Q_PROPERTY(int construction_days READ get_construction_days NOTIFY construction_days_changed)
	Q_PROPERTY(metternich::commodity* commodity READ get_commodity WRITE set_commodity NOTIFY commodity_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(int holding_size READ get_holding_size WRITE set_holding_size NOTIFY holding_size_changed)
	Q_PROPERTY(bool selected READ is_selected WRITE set_selected NOTIFY selected_changed)

public:
	static holding *get_selected_holding()
	{
		return holding::selected_holding;
	}

private:
	static inline holding *selected_holding = nullptr;

public:
	holding(landed_title *barony, holding_type *type, province *province);
	virtual ~holding() override;

	virtual void initialize_history() override;

	void do_day();
	void do_month();

	landed_title *get_barony() const
	{
		return this->barony;
	}

	virtual std::string get_name() const override;

	QString get_name_qstring() const
	{
		return QString::fromStdString(this->get_name());
	}

	std::string get_type_name() const;
	std::string get_titled_name() const;

	QString get_titled_name_qstring() const
	{
		return QString::fromStdString(this->get_titled_name());
	}

	holding_type *get_type() const
	{
		return this->type;
	}

	void set_type(holding_type *type);

	Character *get_owner() const
	{
		return this->owner;
	}

	void set_owner(Character *character)
	{
		if (character == this->get_owner()) {
			return;
		}

		this->owner = character;
		emit owner_changed();
	}

	metternich::province *get_province() const
	{
		return this->province;
	}

	const std::vector<std::unique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(std::unique_ptr<population_unit> &&population_unit);
	QVariantList get_population_units_qvariant_list() const;
	void sort_population_units();
	void remove_empty_population_units();

	int get_population() const
	{
		return this->population;
	}

	void set_population(const int population);

	void change_population(const int change)
	{
		this->set_population(this->get_population() + change);
	}

	void calculate_population();

	int get_base_population_capacity() const
	{
		return this->base_population_capacity;
	}

	void set_base_population_capacity(const int base_population_capacity)
	{
		if (base_population_capacity == this->get_base_population_capacity()) {
			return;
		}

		this->base_population_capacity = base_population_capacity;
		this->calculate_population_capacity();
	}

	void change_base_population_capacity(const int change)
	{
		this->set_base_population_capacity(this->get_base_population_capacity() + change);
	}

	int get_population_capacity_modifier() const
	{
		return this->population_capacity_modifier;
	}

	void set_population_capacity_modifier(const int population_capacity_modifier)
	{
		if (population_capacity_modifier == this->get_population_capacity_modifier()) {
			return;
		}

		this->population_capacity_modifier = population_capacity_modifier;
		this->calculate_population_capacity();
	}

	void change_population_capacity_modifier(const int change)
	{
		this->set_population_capacity_modifier(this->get_population_capacity_modifier() + change);
	}

	int get_population_capacity() const
	{
		return this->population_capacity;
	}

	void set_population_capacity(const int population_capacity)
	{
		if (population_capacity == this->get_population_capacity()) {
			return;
		}

		this->population_capacity = population_capacity;
		emit population_capacity_changed();
		this->calculate_population_growth(); //population growth depends on the population capacity
	}

	void calculate_population_capacity()
	{
		int population_capacity = this->get_base_population_capacity();
		population_capacity *= this->get_population_capacity_modifier();
		population_capacity /= 100;
		population_capacity *= this->get_holding_size();
		population_capacity /= 100;
		this->set_population_capacity(population_capacity);
	}

	int get_base_population_growth() const
	{
		return this->base_population_growth;
	}

	void set_base_population_growth(const int base_population_growth)
	{
		if (base_population_growth == this->get_base_population_growth()) {
			return;
		}

		this->base_population_growth = base_population_growth;
		this->calculate_population_growth();
	}

	void change_base_population_growth(const int change)
	{
		this->set_base_population_growth(this->get_base_population_growth() + change);
	}


	int get_population_growth() const
	{
		return this->population_growth;
	}

	void set_population_growth(const int population_growth)
	{
		if (population_growth == this->get_population_growth()) {
			return;
		}

		this->population_growth = population_growth;
		emit population_growth_changed();
	}

	void calculate_population_growth()
	{
		if (this->get_population() == 0) {
			this->set_population_growth(0);
			return;
		}

		int population_growth = this->get_base_population_growth();
		if (population_growth > 0 && this->get_population() >= this->get_population_capacity()) {
			population_growth = 0;
		}

		this->set_population_growth(population_growth);
	}

	void do_population_growth();
	void check_overpopulation();

	const std::map<PopulationType *, int> &get_population_per_type() const
	{
		return this->population_per_type;
	}

	int get_population_type_population(PopulationType *population_type) const
	{
		auto find_iterator = this->population_per_type.find(population_type);
		if (find_iterator == this->population_per_type.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	const std::map<metternich::culture *, int> &get_population_per_culture() const
	{
		return this->population_per_culture;
	}

	int get_culture_population(metternich::culture *culture) const
	{
		auto find_iterator = this->population_per_culture.find(culture);
		if (find_iterator == this->population_per_culture.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	const std::map<metternich::religion *, int> &get_population_per_religion() const
	{
		return this->population_per_religion;
	}

	int get_religion_population(metternich::religion *religion) const
	{
		auto find_iterator = this->population_per_religion.find(religion);
		if (find_iterator == this->population_per_religion.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	void calculate_population_groups();

	const std::set<Building *> &get_buildings() const
	{
		return this->buildings;
	}

	QVariantList get_buildings_qvariant_list() const;
	Q_INVOKABLE void add_building(Building *building);
	Q_INVOKABLE void remove_building(Building *building);
	void apply_building_effects(const Building *building, const int change);
	std::vector<Building *> get_available_buildings() const;
	QVariantList get_available_buildings_qvariant_list() const;

	Building *get_under_construction_building() const
	{
		return this->under_construction_building;
	}

	void set_under_construction_building(Building *building);

	int get_construction_days() const
	{
		return this->construction_days;
	}

	void set_construction_days(const int construction_days)
	{
		if (construction_days == this->get_construction_days()) {
			return;
		}

		this->construction_days = construction_days;
		emit construction_days_changed();
	}

	void change_construction_days(const int change)
	{
		this->set_construction_days(this->get_construction_days() + change);
	}

	metternich::commodity *get_commodity() const
	{
		return this->commodity;
	}

	void set_commodity(commodity *commodity)
	{
		if (commodity == this->get_commodity()) {
			return;
		}

		this->commodity = commodity;
		emit commodity_changed();
	}

	void generate_commodity();

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

	int get_holding_size() const
	{
		return this->holding_size;
	}

	void set_holding_size(const int holding_size)
	{
		if (holding_size == this->get_holding_size()) {
			return;
		}

		this->holding_size = holding_size;
		emit holding_size_changed();
		this->calculate_population_capacity();
	}

	const std::map<const employment_type *, std::unique_ptr<employment>> &get_employments() const
	{
		return this->employments;
	}

	int get_employment_workforce(const employment_type *employment_type) const;
	void set_employment_workforce(const employment_type *employment_type, const int workforce);

	void change_employment_workforce(const employment_type *employment_type, const int change)
	{
		this->set_employment_workforce(employment_type, this->get_employment_workforce(employment_type) + change);
	}

	bool is_selected() const
	{
		return this->selected;
	}

	void set_selected(const bool selected, const bool notify_engine_interface = true);

	Q_INVOKABLE QVariantList get_population_per_type_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_culture_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_religion_qvariant_list() const;
	Q_INVOKABLE void order_construction(const QVariant &building_variant);

signals:
	void name_changed();
	void titled_name_changed();
	void type_changed();
	void owner_changed();
	void population_units_changed();
	void population_changed();
	void population_capacity_changed();
	void population_growth_changed();
	void population_groups_changed();
	void buildings_changed();
	void available_buildings_changed();
	void under_construction_building_changed();
	void construction_days_changed();
	void commodity_changed();
	void culture_changed();
	void religion_changed();
	void holding_size_changed();
	void selected_changed();

private:
	landed_title *barony = nullptr;
	holding_type *type = nullptr;
	Character *owner = nullptr; //the owner of the holding
	metternich::province *province = nullptr; //the province to which this holding belongs
	std::vector<std::unique_ptr<population_unit>> population_units;
	int base_population_capacity = 0; //the base population capacity
	int population_capacity_modifier = 100; //the population capacity modifier
	int population_capacity = 0; //the population capacity
	int population = 0; //the size of this holding's total population
	int base_population_growth = 0; //the base population growth
	int population_growth = 0; //the population growth, in permyriad (per 10,000)
	std::set<Building *> buildings;
	Building *under_construction_building = nullptr; //the building currently under construction
	int construction_days = 0; //the amount of days remaining to construct the building under construction
	metternich::commodity *commodity = nullptr; //the commodity produced by the holding (if any)
	int holding_size = 100; //the holding size, which affects population capacity (100 = normal size)
	metternich::culture *culture = nullptr; //the holding's culture
	metternich::religion *religion = nullptr; //the holding's religion
	std::set<IdentifiableModifier *> modifiers; //modifiers applied to the holding
	bool selected = false;
	std::map<const employment_type *, std::unique_ptr<employment>> employments; //employments, mapped to their respective employment types
	std::map<PopulationType *, int> population_per_type; //the population for each population type
	std::map<metternich::culture *, int> population_per_culture; //the population for each culture
	std::map<metternich::religion *, int> population_per_religion; //the population for each religion
};

}
