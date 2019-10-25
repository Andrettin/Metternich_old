#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <memory>
#include <vector>

namespace metternich {

class holding;
class holding_slot;
class landed_title;
class population_unit;
class province;

class region : public data_entry, public data_type<region>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList holdings READ get_holdings_qvariant_list)
	Q_PROPERTY(QVariantList provinces READ get_provinces_qvariant_list)
	Q_PROPERTY(QVariantList subregions READ get_subregions_qvariant_list)

public:
	static constexpr const char *class_identifier = "region";
	static constexpr const char *database_folder = "regions";
	static constexpr const char *prefix = "r_";

	static region *add(const std::string &identifier)
	{
		if (identifier.substr(0, 2) != region::prefix) {
			throw std::runtime_error("Invalid identifier for new region: \"" + identifier + "\". Region identifiers must begin with \"" + region::prefix + "\".");
		}

		return data_type<region>::add(identifier);
	}

	static std::set<std::string> get_database_dependencies();

public:
	region(const std::string &identifier);
	virtual ~region() override;

	virtual void initialize() override;
	virtual void initialize_history() override;

	const std::vector<province *> &get_provinces() const
	{
		return this->provinces;
	}

	QVariantList get_provinces_qvariant_list() const;
	Q_INVOKABLE void add_province(province *province);
	Q_INVOKABLE void remove_province(province *province);

	QVariantList get_subregions_qvariant_list() const;

	Q_INVOKABLE void add_subregion(metternich::region *subregion)
	{
		this->subregions.push_back(subregion);
	}

	Q_INVOKABLE void remove_subregion(metternich::region *subregion)
	{
		this->subregions.erase(std::remove(this->subregions.begin(), this->subregions.end(), subregion), this->subregions.end());
	}

	std::vector<holding *> get_holdings() const;

	QVariantList get_holdings_qvariant_list() const;

	Q_INVOKABLE void add_holding(holding_slot *holding_slot)
	{
		this->holding_slots.push_back(holding_slot);
	}

	Q_INVOKABLE void remove_holding(holding_slot *holding_slot)
	{
		this->holding_slots.erase(std::remove(this->holding_slots.begin(), this->holding_slots.end(), holding_slot), this->holding_slots.end());
	}

	const std::vector<std::unique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(std::unique_ptr<population_unit> &&population_unit);

signals:
	void provinces_changed();

private:
	std::vector<province *> provinces;
	std::vector<holding_slot *> holding_slots; //the slots for the holdings contained by this region
	std::vector<region *> subregions; //subregions of this region
	std::vector<std::unique_ptr<population_unit>> population_units; //population units set for this region in history, used during initialization to generate population units in the region's settlements
};

}
