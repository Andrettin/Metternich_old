#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>
#include <vector>

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;

class Character : public NumericDataEntry, DataType<Character, int>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString WRITE SetNameQString)
	Q_PROPERTY(bool female MEMBER Female READ IsFemale)

public:
	Character(const int identifier) : NumericDataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "character";

	void SetNameQString(const QString &name)
	{
		this->Name = name.toStdString();
	}

	const std::string &GetName() const
	{
		return this->Name;
	}

	QString GetNameQString() const
	{
		return QString::fromStdString(this->Name);
	}

	bool IsFemale() const
	{
		return this->Female;
	}

	const Dynasty *GetDynasty() const
	{
		return this->Dynasty;
	}

	const Culture *GetCulture() const
	{
		return this->Culture;
	}

	const Religion *GetReligion() const
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
	bool Female = false;
	const ::Dynasty *Dynasty = nullptr;
	const ::Culture *Culture = nullptr;
	const ::Religion *Religion = nullptr;
	LandedTitle *PrimaryTitle = nullptr;
	std::vector<LandedTitle *> LandedTitles;
	Character *Father = nullptr;
	Character *Mother = nullptr;
	std::vector<Character *> Children;
};
