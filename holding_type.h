#include <string>
#include <vector>

class Building;

class HoldingType
{
public:
	const std::string &GetName() const
	{
		return this->Name;
	}

	const std::vector<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

private:
	std::string Name;
	std::vector<Building *> Buildings;
};
