#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class religion_group : public data_entry, public data_type<religion_group>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)

public:
	static constexpr const char *class_identifier = "religion_group";
	static constexpr const char *database_folder = "religion_groups";

	religion_group(const std::string &identifier) : data_entry(identifier) {}

	virtual void check() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

private:
	QColor color;
};

}
