#include <vector>

class Building;

class HoldingType
{
	const std::vector<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

private:
	std::vector<Building *> Buildings;
};
