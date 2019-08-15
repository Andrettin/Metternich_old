#pragma once

#include "chance_factor.h"
#include "database/data_entry.h"
#include "database/data_type.h"

#include <QString>

#include <string>

namespace Metternich {

class GSMLData;
class Holding;

class Commodity : public DataEntry, public DataType<Commodity>
{
	Q_OBJECT

	Q_PROPERTY(QString icon_path READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)

public:
	static constexpr const char *ClassIdentifier = "commodity";
	static constexpr const char *DatabaseFolder = "commodities";

public:
	Commodity(const std::string &identifier) : DataEntry(identifier) {}

	virtual void ProcessGSMLScope(const GSMLData &scope) override;

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

	int CalculateChance(Holding *holding) const
	{
		//calculate the chance for the commodity to be generated as the produced one for a holding
		return this->Chance.Calculate(holding);
	}

signals:
	void IconPathChanged();

private:
	std::string IconPath;
	ChanceFactor<Holding> Chance; //the chance of the commodity being picked as the one for a given settlement holding
};

}
