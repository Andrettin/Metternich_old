#pragma once

#include "data_entry.h"
#include "data_type.h"

#include <string>
#include <vector>

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;

class Character : public DataEntry<int>, DataType<Character, int>
{
public:
	Character(const int identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "character";

	virtual bool ProcessGSMLProperty(const GSMLProperty &property) override;

	const std::string &GetName() const
	{
		return this->Name;
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
