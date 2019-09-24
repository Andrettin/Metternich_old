#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <string>

namespace metternich {

class religion : public DataEntry, public DataType<religion>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)

public:
	religion(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "religion";
	static constexpr const char *DatabaseFolder = "religions";

	virtual void ProcessGSMLScope(const gsml_data &scope) override;
	virtual void Check() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

private:
	QColor color;
};

}
