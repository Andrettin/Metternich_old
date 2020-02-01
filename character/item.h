#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <memory>
#include <string>

namespace metternich {

class character;

template <typename T>
class modifier;

class item : public data_entry, public data_type<item>
{
	Q_OBJECT

	Q_PROPERTY(QString description READ get_description_qstring CONSTANT)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(QString modifier_effects_string READ get_modifier_effects_string CONSTANT)

public:
	static constexpr const char *class_identifier = "item";
	static constexpr const char *database_folder = "items";

public:
	item(const std::string &identifier);
	virtual ~item() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	virtual void check() const override
	{
		this->get_icon_path(); //throws an exception if the icon isn't found
	}

	std::string get_description() const;

	QString get_description_qstring() const
	{
		return QString::fromStdString(this->get_description());
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

	const std::unique_ptr<metternich::modifier<character>> &get_modifier() const
	{
		return this->modifier;
	}

	QString get_modifier_effects_string() const;

private:
	std::string icon_tag;
	std::unique_ptr<metternich::modifier<character>> modifier; //the modifier applied to characters who have this item equipped
};

}
