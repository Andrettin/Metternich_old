#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class enemy : public data_entry, public data_type<enemy>
{
	Q_OBJECT

	Q_PROPERTY(QString name_plural READ get_name_plural_qstring CONSTANT)
	Q_PROPERTY(int prowess MEMBER prowess READ get_prowess)

public:
	static constexpr const char *class_identifier = "enemy";
	static constexpr const char *database_folder = "enemies";

public:
	enemy(const std::string &identifier) : data_entry(identifier) {}

	virtual void check() const override
	{
		if (this->get_prowess() <= 0) {
			throw std::runtime_error("Enemy \"" + this->get_identifier() + "\" does not have a Prowess greater than 0.");
		}
	}

	std::string get_name_plural() const;

	QString get_name_plural_qstring() const
	{
		return QString::fromStdString(this->get_name_plural());
	}

	int get_prowess() const
	{
		return this->prowess;
	}

private:
	int prowess = 0;
};

}
