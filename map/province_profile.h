#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QObject>

#include <set>

namespace metternich {

class province;

template <typename T>
class condition;

class province_profile : public data_entry, public data_type<province_profile>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "province_profile";
	static constexpr const char *database_folder = "province_profiles";

	static std::set<std::string> get_database_dependencies();

public:
	province_profile(const std::string &identifier);
	virtual ~province_profile() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date) override;
	virtual void process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date) override;

	province *get_province();

private:
	province *province = nullptr;
	std::unique_ptr<condition<metternich::province>> conditions;
};

}
