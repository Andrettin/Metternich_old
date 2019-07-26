#pragma once

#include <string>
#include <vector>

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;

class Character
{
public:
	bool ProcessGSMLProperty(const GSMLProperty &property);

	int GetID() const
	{
		return this->ID;
	}

	const std::string &GetName() const
	{
		return this->Name;
	}

	bool IsFemale() const
	{
		return this->Female;
	}

	Dynasty *GetDynasty() const
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
	int ID = -1;
	std::string Name;
	bool Female = false;
	::Dynasty *Dynasty = nullptr;
	::Culture *Culture = nullptr;
	::Religion *Religion = nullptr;
	LandedTitle *PrimaryTitle = nullptr;
	std::vector<LandedTitle *> LandedTitles;
	Character *Father = nullptr;
	Character *Mother = nullptr;
	std::vector<Character *> Children;
};
