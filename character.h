#include <string>
#include <vector>

class LandedTitle;

class Character
{
public:
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
	LandedTitle *PrimaryTitle = nullptr;
	std::vector<LandedTitle *> LandedTitles;
	Character *Father = nullptr;
	Character *Mother = nullptr;
	std::vector<Character *> Children;
};
