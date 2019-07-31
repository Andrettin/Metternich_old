#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QDateTime>

#include <string>
#include <vector>

namespace Metternich {

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;

class Character : public NumericDataEntry, public DataType<Character, int>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString WRITE SetNameQString)
	Q_PROPERTY(bool female MEMBER Female READ IsFemale)
	Q_PROPERTY(Metternich::Culture* culture MEMBER Culture READ GetCulture)
	Q_PROPERTY(Metternich::Religion* religion MEMBER Religion READ GetReligion)

public:
	Character(const int identifier) : NumericDataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "character";
	static constexpr const char *DatabaseFolder = "characters";

	virtual void ProcessGSMLHistoryProperty(const GSMLProperty &property, const QDateTime &date) override;

	virtual std::string GetName() const override
	{
		return this->Name;
	}

	void SetNameQString(const QString &name)
	{
		this->Name = name.toStdString();
	}

	bool IsAlive() const
	{
		return this->Alive;
	}

	bool IsFemale() const
	{
		return this->Female;
	}

	const Dynasty *GetDynasty() const
	{
		return this->Dynasty;
	}

	Culture *GetCulture() const
	{
		return this->Culture;
	}

	Religion *GetReligion() const
	{
		return this->Religion;
	}

	LandedTitle *GetPrimaryTitle() const
	{
		return this->PrimaryTitle;
	}

	const std::vector<LandedTitle *> &GetLandedTitles() const
	{
		return this->LandedTitles;
	}

private:
	std::string Name;
	bool Alive = false;
	bool Female = false;
	Dynasty *Dynasty = nullptr;
	Culture *Culture = nullptr;
	Religion *Religion = nullptr;
	LandedTitle *PrimaryTitle = nullptr;
	std::vector<LandedTitle *> LandedTitles;
	Character *Father = nullptr;
	Character *Mother = nullptr;
	std::vector<Character *> Children;
	QDateTime BirthDate;
	QDateTime DeathDate;
};

}
