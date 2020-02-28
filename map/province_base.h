#pragma once

#include "database/data_entry.h"

namespace metternich {

class character;
class holding;
class holding_slot;
class holding_type;

//the base class for territories that have holdings
class province_base : public data_entry
{
	Q_OBJECT

	Q_PROPERTY(QVariantList settlement_holding_slots READ get_settlement_holding_slots_qvariant_list NOTIFY settlement_holding_slots_changed)
	Q_PROPERTY(QVariantList settlement_holdings READ get_settlement_holdings_qvariant_list NOTIFY settlement_holdings_changed)
	Q_PROPERTY(metternich::holding_slot* capital_holding_slot READ get_capital_holding_slot WRITE set_capital_holding_slot NOTIFY capital_holding_slot_changed)
	Q_PROPERTY(metternich::holding* capital_holding READ get_capital_holding WRITE set_capital_holding)

public:
	province_base(const std::string &identifier) : data_entry(identifier)
	{
	}

	virtual character *get_owner() const = 0;

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
	void settlement_holding_slots_changed();
	void settlement_holdings_changed();
	void capital_holding_slot_changed();

private:
	std::vector<holding_slot *> settlement_holding_slots;
	std::vector<holding *> settlement_holdings;
	holding_slot *capital_holding_slot = nullptr;
};

}
