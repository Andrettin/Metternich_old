#pragma once

#include "database/data_entry.h"
#include "qunique_ptr.h"
#include "technology/technology_set.h"

namespace metternich {

class character;
class culture;
class holding;
class holding_slot;
class holding_type;
class landed_title;
class population_type;
class population_unit;
class region;
class religion;
class technology;

//the base class for territories that have holdings, i.e. provinces and worlds
class territory : public data_entry
{
	Q_OBJECT

	Q_PROPERTY(metternich::landed_title* county READ get_county WRITE set_county NOTIFY county_changed)
	Q_PROPERTY(metternich::landed_title* duchy READ get_duchy NOTIFY duchy_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_duchy READ get_de_jure_duchy NOTIFY de_jure_duchy_changed)
	Q_PROPERTY(metternich::landed_title* kingdom READ get_kingdom NOTIFY kingdom_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_kingdom READ get_de_jure_kingdom NOTIFY de_jure_kingdom_changed)
	Q_PROPERTY(metternich::landed_title* empire READ get_empire NOTIFY empire_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_empire READ get_de_jure_empire NOTIFY de_jure_empire_changed)
	Q_PROPERTY(metternich::character* owner READ get_owner NOTIFY owner_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(QVariantList settlement_holding_slots READ get_settlement_holding_slots_qvariant_list NOTIFY settlement_holding_slots_changed)
	Q_PROPERTY(QVariantList settlement_holdings READ get_settlement_holdings_qvariant_list NOTIFY settlement_holdings_changed)
	Q_PROPERTY(metternich::holding_slot* capital_holding_slot READ get_capital_holding_slot WRITE set_capital_holding_slot NOTIFY capital_holding_slot_changed)
	Q_PROPERTY(metternich::holding* capital_holding READ get_capital_holding WRITE set_capital_holding)
	Q_PROPERTY(QVariantList palace_holding_slots READ get_palace_holding_slots_qvariant_list CONSTANT)
	Q_PROPERTY(metternich::holding_slot* fort_holding_slot READ get_fort_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* university_holding_slot READ get_university_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* hospital_holding_slot READ get_hospital_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* trading_post_holding_slot READ get_trading_post_holding_slot NOTIFY trading_post_holding_slot_changed)
	Q_PROPERTY(metternich::holding_slot* factory_holding_slot READ get_factory_holding_slot CONSTANT)
	Q_PROPERTY(int population READ get_population WRITE set_population NOTIFY population_changed)
	Q_PROPERTY(QVariantList technologies READ get_technologies_qvariant_list NOTIFY technologies_changed)
	Q_PROPERTY(bool selectable READ is_selectable CONSTANT)

public:
	territory(const std::string &identifier);
	virtual ~territory() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date) override;
	virtual void process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date) override;
	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;
	virtual void check_history() const override;

	virtual void do_month();

	virtual std::string get_identifier_without_prefix() const
	{
		return this->get_identifier();
	}

	virtual std::string get_name() const override;

	std::vector<std::vector<std::string>> get_tag_suffix_list_with_fallbacks() const;

	landed_title *get_county() const
	{
		return this->county;
	}

	virtual void set_county(landed_title *county);

	landed_title *get_duchy() const;
	landed_title *get_de_jure_duchy() const;
	landed_title *get_kingdom() const;
	landed_title *get_de_jure_kingdom() const;
	landed_title *get_empire() const;
	landed_title *get_de_jure_empire() const;

	character *get_owner() const;

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	virtual void set_culture(culture *culture);

	metternich::religion *get_religion() const
	{
		return this->religion;
	}

	virtual void set_religion(religion *religion);

	holding_slot *get_holding_slot(const std::string &holding_slot_str) const;
	virtual void add_holding_slot(holding_slot *holding_slot);

	void create_holding(holding_slot *holding_slot, holding_type *type);
	void destroy_holding(holding_slot *holding_slot);
	void destroy_special_holdings();

	const std::vector<holding_slot *> &get_settlement_holding_slots() const
	{
		return this->settlement_holding_slots;
	}

	QVariantList get_settlement_holding_slots_qvariant_list() const;

	const std::vector<holding *> &get_settlement_holdings() const
	{
		return this->settlement_holdings;
	}

	QVariantList get_settlement_holdings_qvariant_list() const;
	void destroy_settlement_holdings();

	const std::vector<holding_slot *> &get_palace_holding_slots() const
	{
		return this->palace_holding_slots;
	}

	QVariantList get_palace_holding_slots_qvariant_list() const;

	holding_slot *get_fort_holding_slot() const
	{
		return this->fort_holding_slot;
	}

	holding *get_fort_holding() const;

	holding_slot *get_university_holding_slot() const
	{
		return this->university_holding_slot;
	}

	holding *get_university_holding() const;

	holding_slot *get_hospital_holding_slot() const
	{
		return this->hospital_holding_slot;
	}

	holding *get_hospital_holding() const;

	holding_slot *get_trading_post_holding_slot() const
	{
		return this->trading_post_holding_slot;
	}

	std::string get_trading_post_holding_slot_identifier() const;
	void create_trading_post_holding_slot();
	void destroy_trading_post_holding_slot();
	holding *get_trading_post_holding() const;

	virtual bool can_have_trading_post() const
	{
		return this->get_county() != nullptr;
	}

	holding_slot *get_factory_holding_slot() const
	{
		return this->factory_holding_slot;
	}

	holding *get_factory_holding() const;

	holding_slot *get_capital_holding_slot() const
	{
		return this->capital_holding_slot;
	}

	virtual void set_capital_holding_slot(holding_slot *holding_slot);

	holding *get_capital_holding() const;
	void set_capital_holding(holding *holding);

	const std::set<region *> &get_regions() const
	{
		return this->regions;
	}

	void add_region(region *region)
	{
		this->regions.insert(region);
	}

	void remove_region(region *region)
	{
		this->regions.erase(region);
	}

	bool is_in_region(region *region) const
	{
		return this->regions.contains(region);
	}

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

	int get_population_capacity_additive_modifier() const
	{
		return this->population_capacity_additive_modifier;
	}

	void set_population_capacity_additive_modifier(const int population_capacity_modifier);

	void change_population_capacity_additive_modifier(const int change)
	{
		this->set_population_capacity_additive_modifier(this->get_population_capacity_additive_modifier() + change);
	}

	int get_population_capacity_modifier() const
	{
		return this->population_capacity_modifier;
	}

	void set_population_capacity_modifier(const int population_capacity_modifier);

	void change_population_capacity_modifier(const int change)
	{
		this->set_population_capacity_modifier(this->get_population_capacity_modifier() + change);
	}

	int get_population_growth_modifier() const
	{
		return this->population_growth_modifier;
	}

	void set_population_growth_modifier(const int population_capacity_modifier);

	void change_population_growth_modifier(const int change)
	{
		this->set_population_growth_modifier(this->get_population_growth_modifier() + change);
	}

	void calculate_population_groups();

	Q_INVOKABLE QVariantList get_population_per_type_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_culture_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_religion_qvariant_list() const;

	const technology_set &get_technologies() const
	{
		return this->technologies;
	}

	QVariantList get_technologies_qvariant_list() const;

	bool has_technology(technology *technology) const
	{
		return this->technologies.contains(technology);
	}

	Q_INVOKABLE void add_technology(technology *technology);
	Q_INVOKABLE void remove_technology(technology *technology);

	bool is_selectable() const;

	const std::vector<qunique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(qunique_ptr<population_unit> &&population_unit);

signals:
	void county_changed();
	void duchy_changed();
	void de_jure_duchy_changed();
	void kingdom_changed();
	void de_jure_kingdom_changed();
	void empire_changed();
	void de_jure_empire_changed();
	void owner_changed();
	void culture_changed();
	void religion_changed();
	void settlement_holding_slots_changed();
	void settlement_holdings_changed();
	void capital_holding_slot_changed();
	void trading_post_holding_slot_changed();
	void population_changed();
	void population_groups_changed();
	void technologies_changed();

private:
	landed_title *county = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	std::vector<holding_slot *> settlement_holding_slots;
	std::vector<holding *> settlement_holdings;
	std::vector<holding_slot *> palace_holding_slots;
	holding_slot *fort_holding_slot = nullptr;
	holding_slot *university_holding_slot = nullptr;
	holding_slot *hospital_holding_slot = nullptr;
	holding_slot *trading_post_holding_slot = nullptr;
	holding_slot *factory_holding_slot = nullptr;
	holding_slot *capital_holding_slot = nullptr;
	std::set<region *> regions; //the regions to which this territory belongs
	technology_set technologies; //the technologies acquired for the territory
	int population = 0; //the sum of the population of all of the territory's settlement holdings
	int population_capacity_additive_modifier = 0; //the population capacity additive modifier which the territory provides to its holdings
	int population_capacity_modifier = 0; //the population capacity modifier which the territory provides to its holdings
	int population_growth_modifier = 0; //the population growth modifier which the territory provides to its holdings
	std::map<population_type *, int> population_per_type; //the population for each population type
	std::map<metternich::culture *, int> population_per_culture; //the population for each culture
	std::map<metternich::religion *, int> population_per_religion; //the population for each religion
	mutable std::shared_mutex population_groups_mutex;
	std::vector<qunique_ptr<population_unit>> population_units; //population units set for this province in history, used during initialization to generate population units in the province's settlements
};

}
