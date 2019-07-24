#include <set>

class Building;
class HoldingType;

class Holding
{
	const HoldingType *GetType() const
	{
		return this->Type;
	}

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

private:
	const HoldingType *Type = nullptr;
	std::set<Building *> Buildings;
};
