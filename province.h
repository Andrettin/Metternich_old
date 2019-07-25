#include <string>
#include <vector>

class Culture;
class Holding;
class LandedTitle;
class Religion;

class Province
{
public:
	const std::string &GetName() const
	{
		return this->Name;
	}

	LandedTitle *GetCounty() const
	{
		return this->County;
	}

	const Culture *GetCulture() const
	{
		return this->Culture;
	}

	const Religion *GetReligion() const
	{
		return this->Religion;
	}

	Holding *GetCapitalHolding() const
	{
		return this->CapitalHolding;
	}

	const std::vector<Holding *> &GetHoldings() const
	{
		return this->Holdings;
	}

	int GetMaxHoldings() const
	{
		return this->MaxHoldings;
	}

private:
	std::string Name;
	LandedTitle *County = nullptr;
	const ::Culture *Culture = nullptr;
	const ::Religion *Religion = nullptr;
	Holding *CapitalHolding = nullptr;
	std::vector<Holding *> Holdings;
	int MaxHoldings = 1;
};
