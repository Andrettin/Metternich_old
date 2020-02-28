#include "map/province_base.h"

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "qunique_ptr.h"
#include "util/container_util.h"

namespace metternich {

void province_base::add_holding_slot(holding_slot *holding_slot)
{
	switch (holding_slot->get_type()) {
		case holding_slot_type::settlement:
			this->settlement_holding_slots.push_back(holding_slot);
			emit settlement_holding_slots_changed();
			break;
		default:
			throw std::runtime_error("Holding slot \"" + holding_slot->get_identifier() + "\" has an invalid type (" + std::to_string(static_cast<int>(holding_slot->get_type())) + "), but is being added to province base instance \"" + this->get_identifier() + "\".");
	}
}

QVariantList province_base::get_settlement_holding_slots_qvariant_list() const
{
	return container::to_qvariant_list(this->get_settlement_holding_slots());
}

QVariantList province_base::get_settlement_holdings_qvariant_list() const
{
	return container::to_qvariant_list(this->get_settlement_holdings());
}

void province_base::create_holding(holding_slot *holding_slot, holding_type *type)
{
	auto new_holding = make_qunique<holding>(holding_slot, type);
	new_holding->moveToThread(QApplication::instance()->thread());
	holding_slot->set_holding(std::move(new_holding));
	switch (holding_slot->get_type()) {
		case holding_slot_type::settlement:
			this->settlement_holdings.push_back(holding_slot->get_holding());
			emit settlement_holdings_changed();

			if (this->get_capital_holding() == nullptr) {
				this->set_capital_holding(holding_slot->get_holding());
			}

			break;
		case holding_slot_type::fort:
		case holding_slot_type::hospital:
		case holding_slot_type::university:
		case holding_slot_type::trading_post:
		case holding_slot_type::factory:
			holding_slot->get_holding()->set_owner(this->get_owner());
			break;
		default:
			break;
	}
}

void province_base::destroy_holding(holding_slot *holding_slot)
{
	holding *holding = holding_slot->get_holding();

	if (holding_slot->get_type() == holding_slot_type::settlement) {
		if (holding == this->get_capital_holding()) {
			//if the capital holding is being destroyed, set the next holding as the capital, if any exists, or otherwise set the capital holding to null
			if (this->settlement_holdings.size() > 1) {
				this->set_capital_holding(this->settlement_holdings.at(1));
			} else {
				this->set_capital_holding(nullptr);
			}
		}

		this->settlement_holdings.erase(std::remove(this->settlement_holdings.begin(), this->settlement_holdings.end(), holding), this->settlement_holdings.end());
		emit settlement_holdings_changed();
	}

	holding_slot->set_holding(nullptr);
}

void province_base::set_capital_holding_slot(holding_slot *holding_slot)
{
	if (holding_slot == this->get_capital_holding_slot()) {
		return;
	}

	this->capital_holding_slot = holding_slot;
	emit capital_holding_slot_changed();
}

holding *province_base::get_capital_holding() const
{
	if (this->get_capital_holding_slot() != nullptr) {
		return this->get_capital_holding_slot()->get_holding();
	}

	return nullptr;
}

void province_base::set_capital_holding(holding *holding)
{
	if (holding == this->get_capital_holding()) {
		return;
	}

	if (holding != nullptr) {
		this->set_capital_holding_slot(holding->get_slot());
	} else if (!this->get_settlement_holding_slots().empty()) {
		this->set_capital_holding_slot(this->get_settlement_holding_slots().front());
	} else {
		this->set_capital_holding_slot(nullptr);
	}
}

}
