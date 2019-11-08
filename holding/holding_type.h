#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>
#include <vector>

namespace metternich {

class building;
class modifier;
enum class holding_slot_type : int;

class holding_type : public data_entry, public data_type<holding_type>
{
	Q_OBJECT

	Q_PROPERTY(metternich::holding_slot_type slot_type MEMBER slot_type READ get_slot_type)
	Q_PROPERTY(QString portrait_tag READ get_portrait_tag_qstring WRITE set_portrait_tag_qstring)

public:
	static constexpr const char *class_identifier = "holding_type";
	static constexpr const char *database_folder = "holding_types";

	holding_type(const std::string &identifier);
	virtual ~holding_type() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	holding_slot_type get_slot_type() const
	{
		return this->slot_type;
	}

	const std::string &get_portrait_tag() const
	{
		return this->portrait_tag;
	}

	void set_portrait_tag(const std::string &portrait_tag)
	{
		if (portrait_tag == this->get_portrait_tag()) {
			return;
		}

		this->portrait_tag = portrait_tag;
	}

	QString get_portrait_tag_qstring() const
	{
		return QString::fromStdString(this->get_portrait_tag());
	}

	void set_portrait_tag_qstring(const QString &portrait_tag)
	{
		this->set_portrait_tag(portrait_tag.toStdString());
	}

	const std::set<building *> &get_buildings() const
	{
		return this->buildings;
	}

	bool has_building(building *building) const
	{
		return this->buildings.contains(building);
	}

	void add_building(building *building)
	{
		this->buildings.insert(building);
	}

	void remove_building(building *building)
	{
		this->buildings.erase(building);
	}

	const std::unique_ptr<metternich::modifier> &get_modifier() const
	{
		return this->modifier;
	}

private:
	holding_slot_type slot_type;	//the slot type which the holding type occupies
	std::string portrait_tag;
	std::set<building *> buildings;
	std::unique_ptr<metternich::modifier> modifier; //the modifier applied to holdings of this type
};

}
