#pragma once

#include "data_element.h"

#include <string>
#include <vector>

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;

class Character : public DataElement<Character, int>
{
public:
	Character(const int identifier) : Identifier(identifier) {}

	bool ProcessGSMLProperty(const GSMLProperty &property);

	int GetIdentifier() const
	{
		return this->Identifier;
	}

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
	int Identifier = -1;
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
