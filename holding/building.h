#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <vector>

namespace metternich {

class employment_type;
class holding;
class holding_type;
class technology;

template <typename T>
class condition;

class building : public data_entry, public data_type<building>
{
	Q_OBJECT

	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(QVariantList holding_types READ get_holding_types_qvariant_list)
	Q_PROPERTY(int construction_days MEMBER construction_days READ get_construction_days)
	Q_PROPERTY(metternich::employment_type* employment_type MEMBER employment_type READ get_employment_type)
	Q_PROPERTY(int workforce MEMBER workforce READ get_workforce)
	Q_PROPERTY(QVariantList required_technologies READ get_required_technologies_qvariant_list)

public:
	static constexpr const char *class_identifier = "building";
	static constexpr const char *database_folder = "buildings";

	building(const std::string &identifier);
	virtual ~building() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	virtual void check() const override
	{
		this->get_icon_path(); //throws an exception if the icon isn't found
	}

	const std::string &get_icon_tag() const
	{
		if (this->icon_tag.empty()) {
			return this->get_identifier();
		}

		return this->icon_tag;
	}

	void set_icon_tag(const std::string &icon_tag)
	{
		if (icon_tag == this->get_icon_tag()) {
			return;
		}

		this->icon_tag = icon_tag;
	}

	QString get_icon_tag_qstring() const
	{
		return QString::fromStdString(this->get_icon_tag());
	}

	void set_icon_tag_qstring(const QString &icon_tag)
	{
		this->set_icon_tag(icon_tag.toStdString());
	}

	const std::filesystem::path &get_icon_path() const;

	QString get_icon_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_icon_path().string());
	}

	const std::set<holding_type *> &get_holding_types() const
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

	const std::set<technology *> &get_required_technologies() const
	{
		return this->required_technologies;
	}

	QVariantList get_required_technologies_qvariant_list() const;

	Q_INVOKABLE void add_required_technology(technology *technology)
	{
		this->required_technologies.insert(technology);
	}

	Q_INVOKABLE void remove_technology(technology *technology)
	{
		this->required_technologies.erase(technology);
	}

	const condition<holding> *get_preconditions() const
	{
		return this->preconditions.get();
	}

	const condition<holding> *get_conditions() const
	{
		return this->conditions.get();
	}

private:
	std::string icon_tag;
	std::set<holding_type *> holding_types;
	int construction_days = 0; //how many days does it take to construct this building
	metternich::employment_type *employment_type = nullptr;
	int workforce = 0; //how many workers does this building allow for its employment type
	std::set<technology *> required_technologies;
	std::unique_ptr<condition<holding>> preconditions;
	std::unique_ptr<condition<holding>> conditions;
};

}
