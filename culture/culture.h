#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

#include <QColor>

#include <string>
#include <vector>

namespace metternich {

class culture_group;
class Dynasty;

class culture : public culture_base, public data_type<culture>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture_group* culture_group MEMBER culture_group READ get_culture_group NOTIFY culture_group_changed)
	Q_PROPERTY(QColor color MEMBER color READ get_color)

public:
	static constexpr const char *class_identifier = "culture";
	static constexpr const char *database_folder = "cultures";

	culture(const std::string &identifier) : culture_base(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	metternich::culture_group *get_culture_group() const
	{
		return this->culture_group;
	}

	const QColor &get_color() const
	{
		return this->color;
	}

	void add_dynasty(Dynasty *dynasty)
	{
		this->dynasties.push_back(dynasty);
	}

	std::string generate_male_name() const;
	std::string generate_female_name() const;
	std::string generate_dynasty_name() const;

signals:
	void culture_group_changed();

private:
	metternich::culture_group *culture_group = nullptr;
	QColor color;
	std::vector<Dynasty *> dynasties;
};

}
