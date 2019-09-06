#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <string>

namespace metternich {

class Religion : public DataEntry, public DataType<Religion>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER Color READ GetColor)

public:
	Religion(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "religion";
	static constexpr const char *DatabaseFolder = "religions";

	virtual void ProcessGSMLScope(const gsml_data &scope) override;
	virtual void Check() const override;

	const QColor &GetColor() const
	{
		return this->Color;
	}

private:
	QColor Color;
};

}
