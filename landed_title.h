#pragma once

#include <string>

class Character;

class LandedTitle
{
public:
	static constexpr const char *ClassIdentifier = "landed_title";

	const std::string &GetName() const
	{
		return this->Name;
	}

	Character *GetOwner() const
	{
		return this->Owner;
	}

private:
	std::string Name;
	Character *Owner = nullptr;
};
