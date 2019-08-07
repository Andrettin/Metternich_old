#pragma once

#include <QObject>

#include <set>
#include <string>

namespace Metternich {

class Building;
class HoldingType;
class LandedTitle;
class Province;

class Holding : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)
	Q_PROPERTY(Metternich::HoldingType* type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::LandedTitle* barony READ GetBarony NOTIFY BaronyChanged)

public:
	Holding(LandedTitle *barony, HoldingType *type, Province *province);
	~Holding();

	LandedTitle *GetBarony() const
	{
		return this->Barony;
	}

	std::string GetName() const;

	QString GetNameQString() const
	{
		return QString::fromStdString(this->GetName());
	}

	HoldingType *GetType() const
	{
		return this->Type;
	}

	void SetType(HoldingType *type)
	{
		if (type == this->GetType()) {
			return;
		}

		this->Type = type;
		emit TypeChanged();
	}

	Province *GetProvince() const
	{
		return this->Province;
	}

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

signals:
	void NameChanged();
	void TypeChanged();
	void BaronyChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::set<Building *> Buildings;
};

}
