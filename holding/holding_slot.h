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
class province_profile;
enum class holding_slot_type : int;

class holding_slot : public data_entry, public data_type<holding_slot>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring NOTIFY name_changed)
	Q_PROPERTY(metternich::province* province READ get_province WRITE set_province)
	Q_PROPERTY(metternich::province_profile* province_profile MEMBER province_profile)
	Q_PROPERTY(metternich::holding_slot_type type READ get_type WRITE set_type)
	Q_PROPERTY(bool settlement READ is_settlement CONSTANT)
	Q_PROPERTY(metternich::landed_title* barony READ get_barony WRITE set_barony NOTIFY barony_changed)
	Q_PROPERTY(metternich::holding* holding READ get_holding NOTIFY holding_changed)
	Q_PROPERTY(QVariantList available_commodities READ get_available_commodities_qvariant_list NOTIFY available_commodities_changed)
	Q_PROPERTY(bool population_distribution_allowed MEMBER population_distribution_allowed READ is_population_distribution_allowed)

public:
	static constexpr const char *class_identifier = "holding_slot";
	static constexpr const char *database_folder = "holding_slots";
	static constexpr const char *prefix = "h_";

	static holding_slot *add(const std::string &identifier)
	{
		if (identifier.substr(0, 2) != holding_slot::prefix) {
			throw std::runtime_error("Invalid identifier for new holding slot: \"" + identifier + "\". Holding slot identifiers must begin with \"" + holding_slot::prefix + "\".");
		}

		holding_slot *holding_slot = data_type<metternich::holding_slot>::add(identifier);
		return holding_slot;
	}

	holding_slot(const std::string &identifier);
	virtual ~holding_slot() override;

	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;

	virtual std::string get_name() const override;

	QString get_name_qstring() const
	{
		return QString::fromStdString(this->get_name());
	}

	std::vector<std::vector<std::string>> get_tag_suffix_list_with_fallbacks() const;

	holding_slot_type get_type() const
	{
		return this->type;
	}

	void set_type(const holding_slot_type type)
	{
		this->type = type;
	}

	bool is_settlement() const;

	landed_title *get_barony() const
	{
		return this->barony;
	}

	void set_barony(landed_title *barony);

	holding *get_holding() const
	{
		return this->holding.get();
	}

	void set_holding(std::unique_ptr<holding> &&holding);

	province *get_province() const
	{
		return this->province;
	}

	void set_province(province *province);

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

	bool is_population_distribution_allowed() const
	{
		return this->population_distribution_allowed;
	}

signals:
	void barony_changed();
	void holding_changed();
	void available_commodities_changed();

private:
	holding_slot_type type; //the type of the holding slot
	landed_title *barony = nullptr; //the barony corresponding to this holding slot
	std::unique_ptr<holding> holding; //the holding built on this slot, if any
	province *province = nullptr; //to which province this holding slot belongs
	province_profile *province_profile = nullptr;
	std::vector<metternich::commodity *> available_commodities; //the commodities available for production by the holding (if any)
	bool population_distribution_allowed = true;
};

}
