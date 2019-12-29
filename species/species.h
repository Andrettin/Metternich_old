#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <set>

namespace metternich {

class species : public data_entry, public data_type<species>
{
	Q_OBJECT

	Q_PROPERTY(QString name_plural READ get_name_plural_qstring CONSTANT)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(bool sapient MEMBER sapient READ is_sapient)
	Q_PROPERTY(int average_weight MEMBER average_weight READ get_average_weight)
	Q_PROPERTY(QVariantList evolutions READ get_evolutions_qvariant_list)

public:
	static constexpr const char *class_identifier = "species";
	static constexpr const char *database_folder = "species";

	species(const std::string &identifier) : data_entry(identifier) {}

	virtual void check() const override
	{
		if (this->get_average_weight() == 0 && !this->is_sapient()) {
			throw std::runtime_error("Wildlife species \"" + this->get_identifier() + "\" has no average weight.");
		}
	}

	std::string get_name_plural() const;

	QString get_name_plural_qstring() const
	{
		return QString::fromStdString(this->get_name_plural());
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

	bool is_sapient() const
	{
		return this->sapient;
	}

	int get_average_weight() const
	{
		return this->average_weight;
	}

	const std::set<species *> &get_evolutions() const
	{
		return this->evolutions;
	}

	QVariantList get_evolutions_qvariant_list() const;

	Q_INVOKABLE void add_evolution(species *evolution)
	{
		this->evolutions.insert(evolution);
	}

	Q_INVOKABLE void remove_evolution(species *evolution)
	{
		this->evolutions.erase(evolution);
	}

private:
	std::string icon_tag;
	bool sapient = false; //whether the species is sapient
	int average_weight = 0; //the average weight for individuals of this species, in kg
	std::set<species *> evolutions;
};

}
