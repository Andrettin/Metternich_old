#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <vector>

namespace Metternich {

class Condition;
class Holding;
class HoldingType;

class Building : public DataEntry, public DataType<Building>
{
	Q_OBJECT

	Q_PROPERTY(QString icon READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)
	Q_PROPERTY(QString icon_path READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)
	Q_PROPERTY(QVariantList holding_types READ GetHoldingTypesQVariantList)
	Q_PROPERTY(int construction_days MEMBER ConstructionDays READ GetConstructionDays)

public:
	static constexpr const char *ClassIdentifier = "building";
	static constexpr const char *DatabaseFolder = "buildings";

	Building(const std::string &identifier);
	virtual ~Building() override;

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

	const std::vector<HoldingType *> &GetHoldingTypes() const
	{
		return this->HoldingTypes;
	}

	QVariantList GetHoldingTypesQVariantList() const;
	Q_INVOKABLE void AddHoldingType(HoldingType *holding_type);
	Q_INVOKABLE void RemoveHoldingType(HoldingType *holding_type);

	int GetConstructionDays() const
	{
		return this->ConstructionDays;
	}

	bool IsAvailableForHolding(const Holding *holding) const;
	bool IsBuildableInHolding(const Holding *holding) const;

signals:
	void IconPathChanged();

private:
	std::string IconPath;
	std::vector<HoldingType *> HoldingTypes;
	int ConstructionDays = 0; //how many days does it take to construct this building
	std::unique_ptr<Condition> Preconditions;
	std::unique_ptr<Condition> Conditions;
};

}
