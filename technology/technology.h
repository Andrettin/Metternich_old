#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class technology_category;

class technology : public data_entry, public data_type<technology>
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology_category* category MEMBER category READ get_category)
	Q_PROPERTY(QVariantList required_technologies READ get_required_technologies_qvariant_list)

public:
	technology(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "technology";
	static constexpr const char *database_folder = "technologies";

	technology_category *get_category() const
	{
		return this->category;
	}

	const std::set<technology *> &get_required_technologies() const
	{
		return this->required_technologies;
	}

	QVariantList get_required_technologies_qvariant_list() const;

	Q_INVOKABLE void add_required_technology(technology *technology)
	{
		this->required_technologies.insert(technology);
	}

	Q_INVOKABLE void remove_technology(technology *technology)
	{
		this->required_technologies.erase(technology);
	}

private:
	technology_category *category = nullptr;
	std::set<technology *> required_technologies;
};

}
