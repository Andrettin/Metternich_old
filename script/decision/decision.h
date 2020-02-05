#pragma once

#include "database/data_entry.h"

#include <QString>

namespace metternich {

class decision : public data_entry
{
	Q_OBJECT

	Q_PROPERTY(QString description READ get_description_qstring CONSTANT)

public:
	decision(const std::string &identifier) : data_entry(identifier) {}

	std::string get_description() const;

	QString get_description_qstring() const
	{
		return QString::fromStdString(this->get_description());
	}
};

}
