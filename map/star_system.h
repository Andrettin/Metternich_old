#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class world;

class star_system : public data_entry, public data_type<star_system>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList worlds READ get_worlds_qvariant_list CONSTANT)

public:
	static constexpr const char *class_identifier = "star_system";
	static constexpr const char *database_folder = "star_systems";

public:
	star_system(const std::string &identifier) : data_entry(identifier)
	{
	}

	const std::vector<world *> &get_worlds() const
	{
		return this->worlds;
	}

	QVariantList get_worlds_qvariant_list() const;

	void add_world(world *world)
	{
		this->worlds.push_back(world);
	}

	void remove_world(world *world);

private:
	std::vector<world *> worlds;
};

}
