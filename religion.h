#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <string>

namespace metternich {

class religion : public data_entry, public data_type<religion>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)

public:
	religion(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "religion";
	static constexpr const char *database_folder = "religions";

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

private:
	QColor color;
};

}
