#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class world_type : public data_entry, public data_type<world_type>
{
	Q_OBJECT

	Q_PROPERTY(bool star MEMBER star READ is_star)
	Q_PROPERTY(bool ethereal MEMBER ethereal READ is_ethereal)

public:
	static constexpr const char *class_identifier = "world_type";
	static constexpr const char *database_folder = "world_types";

	world_type(const std::string &identifier) : data_entry(identifier)
	{
	}

	bool is_star() const
	{
		return this->star;
	}

	bool is_ethereal() const
	{
		return this->ethereal;
	}

private:
	bool star = false;
	bool ethereal = false; //whether the world is an ethereal one (i.e. Asgard)
};

}
