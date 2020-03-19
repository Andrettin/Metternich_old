#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <set>
#include <string>

namespace metternich {

enum class troop_category;

class troop_type final : public data_entry, public data_type<troop_type>
{
	Q_OBJECT

	Q_PROPERTY(metternich::troop_category category MEMBER category READ get_category)
	Q_PROPERTY(QString category_name READ get_category_name_qstring CONSTANT)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(int attack MEMBER attack READ get_attack)
	Q_PROPERTY(int defense MEMBER defense READ get_defense)

public:
	static constexpr const char *class_identifier = "troop_type";
	static constexpr const char *database_folder = "troop_types";

public:
	troop_type(const std::string &identifier);

	virtual void check() const override;

	troop_category get_category() const
	{
		return this->category;
	}

	std::string get_category_name() const;

	QString get_category_name_qstring() const
	{
		return QString::fromStdString(this->get_category_name());
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

	int get_attack() const
	{
		return this->attack;
	}

	int get_defense() const
	{
		return this->defense;
	}

private:
	troop_category category;
	std::string icon_tag;
	int attack = 0;
	int defense = 0;
};

}
