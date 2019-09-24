#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QString>

#include <string>

namespace metternich {

class ChanceFactor;
class gsml_data;
class holding;

class commodity : public data_entry, public DataType<commodity>
{
	Q_OBJECT

	Q_PROPERTY(int base_price MEMBER base_price READ get_base_price)
	Q_PROPERTY(QString icon READ get_icon_path_qstring WRITE set_icon_path_qstring NOTIFY icon_path_changed)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring WRITE set_icon_path_qstring NOTIFY icon_path_changed)

public:
	static constexpr const char *ClassIdentifier = "commodity";
	static constexpr const char *DatabaseFolder = "commodities";

public:
	commodity(const std::string &identifier);
	virtual ~commodity() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	virtual void check() const override
	{
		if (this->get_base_price() == 0) {
			throw std::runtime_error("Commodity \"" + this->get_identifier() + "\" has no base price.");
		}
	}

	int get_base_price() const
	{
		return this->base_price;
	}

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

	int calculate_chance(holding *holding) const;

signals:
	void icon_path_changed();

private:
	int base_price = 0; //the commodity's base price
	std::string icon_path;
	std::unique_ptr<ChanceFactor> chance; //the chance of the commodity being picked as the one for a given settlement holding
};

}
