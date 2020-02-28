#pragma once

#include "database/data_entry.h"

namespace metternich {

class character;
class culture;
class holding;
class holding_slot;
class holding_type;
class landed_title;
class religion;

//the base class for territories that have holdings
class province_base : public data_entry
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

public:
	province_base(const std::string &identifier) : data_entry(identifier)
	{
	}

	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;
	virtual void check_history() const override;

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

	virtual void add_holding_slot(holding_slot *holding_slot);

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

	void create_holding(holding_slot *holding_slot, holding_type *type);
	void destroy_holding(holding_slot *holding_slot);

	holding_slot *get_capital_holding_slot() const
	{
		return this->capital_holding_slot;
	}

	virtual void set_capital_holding_slot(holding_slot *holding_slot);

	holding *get_capital_holding() const;
	void set_capital_holding(holding *holding);

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

private:
	landed_title *county = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	std::vector<holding_slot *> settlement_holding_slots;
	std::vector<holding *> settlement_holdings;
	holding_slot *capital_holding_slot = nullptr;
};

}
