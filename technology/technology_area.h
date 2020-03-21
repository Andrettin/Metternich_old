#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class technology;
enum class technology_category;

class technology_area final : public data_entry, public data_type<technology_area>
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology_category category MEMBER category READ get_category)
	Q_PROPERTY(QString category_name READ get_category_name_qstring CONSTANT)
	Q_PROPERTY(QVariantList technologies READ get_technologies_qvariant_list CONSTANT)

public:
	static constexpr const char *class_identifier = "technology_area";
	static constexpr const char *database_folder = "technology_areas";

	static std::vector<technology_area *> get_all_sorted();

	technology_area(const std::string &identifier);

	virtual void check() const override;

	technology_category get_category() const
	{
		return this->category;
	}

	std::string get_category_name() const;

	QString get_category_name_qstring() const
	{
		return QString::fromStdString(this->get_category_name());
	}

	QVariantList get_technologies_qvariant_list() const;

	void add_technology(technology *technology)
	{
		this->technologies.push_back(technology);
	}

	void remove_technology(technology *technology);

	int get_min_level() const;

private:
	technology_category category;
	std::vector<technology *> technologies;
};

}
