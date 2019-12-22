#pragma once

#include "database/simple_data_type.h"
#include "population/population_unit_base.h"

#include <QObject>

#include <set>

namespace metternich {

class species;

class wildlife_unit : public population_unit_base, public simple_data_type<wildlife_unit>
{
	Q_OBJECT

	Q_PROPERTY(metternich::species* species MEMBER species READ get_species NOTIFY species_changed)

public:
	static constexpr const char *database_folder = "wildlife_units";

	static void process_history_database();

	wildlife_unit(species *species) : species(species)
	{
		connect(this, &wildlife_unit::species_changed, this, &wildlife_unit::icon_path_changed);
	}

	void do_month();

	species *get_species() const
	{
		return this->species;
	}

	void subtract_existing_sizes();
	void subtract_existing_sizes_in_province(const metternich::province *province);
	void subtract_existing_sizes_in_provinces(const std::set<metternich::province *> &provinces);
	bool can_distribute_to_province(const metternich::province *province) const;
	void distribute_to_provinces(const std::set<metternich::province *> &provinces);

	virtual const std::filesystem::path &get_icon_path() const override;

signals:
	void species_changed();

private:
	species *species = nullptr;
};

}
