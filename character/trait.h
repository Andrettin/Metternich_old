#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <set>
#include <string>

namespace metternich {

class character;

template <typename T>
class modifier;

class trait : public data_entry, public data_type<trait>
{
	Q_OBJECT

	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(bool personality READ is_personality WRITE set_personality)
	Q_PROPERTY(QString modifier_effects_string READ get_modifier_effects_string CONSTANT)

public:
	static constexpr const char *class_identifier = "trait";
	static constexpr const char *database_folder = "traits";

	static const std::set<trait *> &get_personality_traits()
	{
		return trait::personality_traits;
	}

private:
	static inline std::set<trait *> personality_traits;

public:
	trait(const std::string &identifier);
	virtual ~trait() override;

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

	bool is_personality() const
	{
		return this->personality;
	}

	void set_personality(const bool personality)
	{
		if (personality == this->is_personality()) {
			return;
		}

		this->personality = personality;

		if (personality) {
			trait::personality_traits.insert(this);
		} else {
			trait::personality_traits.erase(this);
		}
	}

	const std::unique_ptr<metternich::modifier<character>> &get_modifier() const
	{
		return this->modifier;
	}

	QString get_modifier_effects_string() const;

private:
	std::string icon_tag;
	bool personality = false; //whether this is a personality trait
	std::unique_ptr<metternich::modifier<character>> modifier; //the modifier applied to characters with this trait
};

}
