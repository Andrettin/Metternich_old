#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class culture;

class dynasty final : public data_entry, public data_type<dynasty>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture* culture MEMBER culture READ get_culture)

public:
	dynasty(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "dynasty";
	static constexpr const char *database_folder = "dynasties";

	static dynasty *generate(culture *culture);

	virtual void initialize() override;

	virtual std::string get_name() const override
	{
		if (!this->name.empty()) {
			return this->name;
		}

		return data_entry::get_name();
	}

	void set_name(const std::string &name)
	{
		if (name == this->name) {
			return;
		}

		this->name = name;
		emit name_changed();
	}

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

private:
	std::string name;
	metternich::culture *culture = nullptr;
};

}
