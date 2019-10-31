#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>
#include <vector>

namespace metternich {

class commodity;
class holding;
class landed_title;
class province;

class holding_slot : public data_entry, public data_type<holding_slot>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring NOTIFY name_changed)
	Q_PROPERTY(metternich::landed_title* barony READ get_barony WRITE set_barony NOTIFY barony_changed)
	Q_PROPERTY(metternich::holding* holding READ get_holding WRITE set_holding NOTIFY holding_changed)
	Q_PROPERTY(bool settlement MEMBER settlement READ is_settlement)
	Q_PROPERTY(QVariantList available_commodities READ get_available_commodities_qvariant_list NOTIFY available_commodities_changed)

public:
	static constexpr const char *class_identifier = "holding_slot";
	static constexpr const char *database_folder = "";
	static constexpr const char *prefix = "h_";

	static holding_slot *add(const std::string &identifier, province *province = nullptr)
	{
		if (identifier.substr(0, 2) != holding_slot::prefix) {
			throw std::runtime_error("Invalid identifier for new holding slot: \"" + identifier + "\". Holding slot identifiers must begin with \"" + holding_slot::prefix + "\".");
		}

		holding_slot *holding_slot = data_type<metternich::holding_slot>::add(identifier);
		holding_slot->province = province;
		return holding_slot;
	}

	holding_slot(const std::string &identifier) : data_entry(identifier) {}

	virtual void initialize() override;

	virtual std::string get_name() const override;

	QString get_name_qstring() const
	{
		return QString::fromStdString(this->get_name());
	}

	landed_title *get_barony() const
	{
		return this->barony;
	}

	void set_barony(landed_title *barony);

	holding *get_holding() const
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

	province *get_province() const
	{
		return this->province;
	}

	bool is_settlement() const
	{
		return this->settlement;
	}


	const std::vector<commodity *> &get_available_commodities() const
	{
		return this->available_commodities;
	}

	QVariantList get_available_commodities_qvariant_list() const;

	Q_INVOKABLE void add_available_commodity(commodity *commodity)
	{
		this->available_commodities.push_back(commodity);
		emit available_commodities_changed();
	}

	Q_INVOKABLE void remove_available_commodity(commodity *commodity)
	{
		this->available_commodities.erase(std::remove(this->available_commodities.begin(), this->available_commodities.end(), commodity), this->available_commodities.end());
		emit available_commodities_changed();
	}

	void generate_available_commodity();

signals:
	void barony_changed();
	void holding_changed();
	void available_commodities_changed();

private:
	landed_title *barony = nullptr; //the barony corresponding to this holding slot
	holding *holding = nullptr; //the holding built on this slot, if any
	province *province = nullptr; //to which province this holding slot belongs
	bool settlement = false; //whether the holding slot is a settlement one
	std::vector<metternich::commodity *> available_commodities; //the commodities available for production by the holding (if any)
};

}