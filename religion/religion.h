#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <string>

namespace metternich {

class religion_group;

class religion final : public data_entry, public data_type<religion>
{
	Q_OBJECT

	Q_PROPERTY(metternich::religion_group* group MEMBER group READ get_group NOTIFY group_changed)
	Q_PROPERTY(QColor color MEMBER color READ get_color)

public:
	religion(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "religion";
	static constexpr const char *database_folder = "religions";

	virtual void check() const override;

	religion_group *get_group() const
	{
		return this->group;
	}

	const QColor &get_color() const
	{
		return this->color;
	}

signals:
	void group_changed();

private:
	religion_group *group = nullptr;
	QColor color;
};

}
