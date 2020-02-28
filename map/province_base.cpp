#include "map/province_base.h"

#include "culture/culture.h"
#include "defines.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "landed_title/landed_title.h"
#include "qunique_ptr.h"
#include "religion/religion.h"
#include "util/container_util.h"

namespace metternich {

void province_base::initialize()
{
	if (this->get_county() != nullptr) {
		connect(this->get_county(), &landed_title::holder_changed, this, &province_base::owner_changed);

	}

	data_entry_base::initialize();
}

void province_base::initialize_history()
{
	if (this->get_capital_holding_slot() == nullptr && !this->get_settlement_holding_slots().empty()) {
		//set the first settlement holding slot as the capital if none has been set
		this->set_capital_holding_slot(this->get_settlement_holding_slots().front());
	}

	//ensure the province's settlement holding slots have been initialized, so that its culture and religion will be calculated correctly
	for (holding_slot *settlement_holding_slot : this->get_settlement_holding_slots()) {
		if (!settlement_holding_slot->is_history_initialized()) {
			settlement_holding_slot->initialize_history();
		}
	}

	data_entry_base::initialize_history();
}

void province_base::check() const
{
	if (this->get_county() != nullptr) {
		if (this->get_settlement_holding_slots().empty()) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has a county (not being a wasteland or water zone), but has no settlement holding slots.");
		}
	}

	if (static_cast<int>(this->get_settlement_holding_slots().size()) > defines::get()->get_max_settlement_slots_per_province()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has " + std::to_string(this->get_settlement_holding_slots().size()) + " settlement slots, but the maximum settlement slots per province is set to " + std::to_string(defines::get()->get_max_settlement_slots_per_province()) + ".");
	}
}

void province_base::check_history() const
{
	if (this->get_county() != nullptr && !this->get_settlement_holdings().empty()) {
		if (this->get_culture() == nullptr) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no culture.");
		}

		if (this->get_religion() == nullptr) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no religion.");
		}
	}

	this->check();
}

void province_base::set_county(landed_title *county)
{
	if (county == this->get_county()) {
		return;
	}

	this->county = county;
	emit county_changed();
}

landed_title *province_base::get_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_duchy();
	}

	return nullptr;
}

landed_title *province_base::get_de_jure_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_duchy();
	}

	return nullptr;
}

landed_title *province_base::get_kingdom() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_kingdom();
	}

	return nullptr;
}

landed_title *province_base::get_de_jure_kingdom() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_kingdom();
	}

	return nullptr;
}

landed_title *province_base::get_empire() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_empire();
	}

	return nullptr;
}

landed_title *province_base::get_de_jure_empire() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_empire();
	}

	return nullptr;
}

character *province_base::get_owner() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_holder();
	}

	return nullptr;
}


void province_base::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	if (culture == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Tried to set the culture of province base \"" + this->get_identifier() + "\" to null, despite it having an owner.");
	}

	this->culture = culture;
	emit culture_changed();
}

void province_base::set_religion(metternich::religion *religion)
{
	if (religion == this->get_religion()) {
		return;
	}

	if (religion == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Tried to set the religion of province \"" + this->get_identifier() + "\" to null, despite it having an owner.");
	}

	this->religion = religion;
	emit religion_changed();
}

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
