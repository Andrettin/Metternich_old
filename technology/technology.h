#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class technology_category;

class technology final : public data_entry, public data_type<technology>
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology_category* category MEMBER category READ get_category)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(QVariantList required_technologies READ get_required_technologies_qvariant_list)

public:
	technology(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "technology";
	static constexpr const char *database_folder = "technologies";

	virtual void check() const override
	{
		this->get_icon_path(); //throws an exception if the icon isn't found
	}

	technology_category *get_category() const
	{
		return this->category;
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

	const std::set<technology *> &get_required_technologies() const
	{
		return this->required_technologies;
	}

	QVariantList get_required_technologies_qvariant_list() const;

	bool requires_technology(technology *technology) const
	{
		if (this->required_technologies.contains(technology)) {
			return true;
		}

		for (metternich::technology *required_technology : this->get_required_technologies()) {
			if (required_technology->requires_technology(technology)) {
				return true;
			}
		}

		return false;
	}

	Q_INVOKABLE void add_required_technology(technology *technology)
	{
		this->required_technologies.insert(technology);
	}

	Q_INVOKABLE void remove_required_technology(technology *technology)
	{
		this->required_technologies.erase(technology);
	}

private:
	technology_category *category = nullptr;
	std::string icon_tag;
	std::set<technology *> required_technologies;
};

}
