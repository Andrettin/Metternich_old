#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QString>

#include <string>

namespace metternich {

class gsml_data;
class holding_slot;

template <typename T>
class chance_factor;

class commodity final : public data_entry, public data_type<commodity>
{
	Q_OBJECT

	Q_PROPERTY(int base_price MEMBER base_price READ get_base_price)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)

public:
	static constexpr const char *class_identifier = "commodity";
	static constexpr const char *database_folder = "commodities";

public:
	commodity(const std::string &identifier);
	virtual ~commodity() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	virtual void check() const override
	{
		if (this->get_base_price() == 0) {
			throw std::runtime_error("Commodity \"" + this->get_identifier() + "\" has no base price.");
		}

		this->get_icon_path(); //throws an exception if the icon isn't found
	}

	int get_base_price() const
	{
		return this->base_price;
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

	const chance_factor<holding_slot> *get_chance_factor() const
	{
		return this->chance.get();
	}

private:
	int base_price = 0; //the commodity's base price
	std::string icon_tag;
	std::unique_ptr<chance_factor<holding_slot>> chance; //the chance of the commodity being picked as the one for a given settlement holding
};

}
