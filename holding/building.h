#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <vector>

namespace metternich {

class condition;
class employment_type;
class holding;
class holding_type;

class building : public data_entry, public data_type<building>
{
	Q_OBJECT

	Q_PROPERTY(QString icon READ get_icon_path_qstring WRITE set_icon_path_qstring NOTIFY icon_path_changed)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring WRITE set_icon_path_qstring NOTIFY icon_path_changed)
	Q_PROPERTY(QVariantList holding_types READ get_holding_types_qvariant_list)
	Q_PROPERTY(int construction_days MEMBER construction_days READ get_construction_days)
	Q_PROPERTY(metternich::employment_type* employment_type MEMBER employment_type READ get_employment_type)
	Q_PROPERTY(int workforce MEMBER workforce READ get_workforce)

public:
	static constexpr const char *class_identifier = "building";
	static constexpr const char *database_folder = "buildings";

	building(const std::string &identifier);
	virtual ~building() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	const std::string &get_icon_path() const
	{
		return this->icon_path;
	}

	QString get_icon_path_qstring() const
	{
		return QString::fromStdString(this->icon_path);
	}

	void set_icon_path(const std::string &icon_path)
	{
		if (icon_path == this->get_icon_path()) {
			return;
		}

		this->icon_path = icon_path;
		emit icon_path_changed();
	}

	void set_icon_path_qstring(const QString &icon_path)
	{
		this->set_icon_path(icon_path.toStdString());
	}

	const std::vector<holding_type *> &get_holding_types() const
	{
		return this->holding_types;
	}

	QVariantList get_holding_types_qvariant_list() const;
	Q_INVOKABLE void add_holding_type(holding_type *holding_type);
	Q_INVOKABLE void remove_holding_type(holding_type *holding_type);

	int get_construction_days() const
	{
		return this->construction_days;
	}

	metternich::employment_type *get_employment_type() const
	{
		return this->employment_type;
	}

	int get_workforce() const
	{
		return this->workforce;
	}

	bool is_available_for_holding(const holding *holding) const;
	bool is_buildable_in_holding(const holding *holding) const;

signals:
	void icon_path_changed();

private:
	std::string icon_path;
	std::vector<holding_type *> holding_types;
	int construction_days = 0; //how many days does it take to construct this building
	std::unique_ptr<condition> preconditions;
	std::unique_ptr<condition> conditions;
	metternich::employment_type *employment_type = nullptr;
	int workforce = 0; //how many workers does this building allow for its employment type
};

}
