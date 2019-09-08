#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QString>

#include <string>

namespace metternich {

class ChanceFactor;
class gsml_data;
class holding;

class Commodity : public DataEntry, public DataType<Commodity>
{
	Q_OBJECT

	Q_PROPERTY(int base_price MEMBER base_price READ get_base_price)
	Q_PROPERTY(QString icon READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)
	Q_PROPERTY(QString icon_path READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)

public:
	static constexpr const char *ClassIdentifier = "commodity";
	static constexpr const char *DatabaseFolder = "commodities";

public:
	Commodity(const std::string &identifier);
	virtual ~Commodity() override;

	virtual void ProcessGSMLScope(const gsml_data &scope) override;

	virtual void Check() const override
	{
		if (this->get_base_price() == 0) {
			throw std::runtime_error("Commodity \"" + this->GetIdentifier() + "\" has no base price.");
		}
	}

	int get_base_price() const
	{
		return this->base_price;
	}

	const std::string &GetIconPath() const
	{
		return this->IconPath;
	}

	QString GetIconPathQString() const
	{
		return QString::fromStdString(this->IconPath);
	}

	void SetIconPath(const std::string &icon_path)
	{
		if (icon_path == this->GetIconPath()) {
			return;
		}

		this->IconPath = icon_path;
		emit IconPathChanged();
	}

	void SetIconPathQString(const QString &icon_path)
	{
		this->SetIconPath(icon_path.toStdString());
	}

	int CalculateChance(holding *holding) const;

signals:
	void IconPathChanged();

private:
	int base_price = 0; //the commodity's base price
	std::string IconPath;
	std::unique_ptr<ChanceFactor> Chance; //the chance of the commodity being picked as the one for a given settlement holding
};

}
