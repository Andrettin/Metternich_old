#pragma once

#include "data_element.h"

#include <QColor>

#include <string>
#include <vector>

class CSVData;
class Culture;
class Holding;
class LandedTitle;
class Religion;

class Province : public DataElement<Province>
{
public:
	Province(const std::string identifier) : Identifier(identifier) {}

	static constexpr const char *ClassIdentifier = "province";
	static constexpr const char *DatabaseFolder = "provinces";

	bool ProcessGSMLProperty(const GSMLProperty &property);
	bool ProcessGSMLData(const GSMLData &data);

	const std::string &GetIdentifier() const
	{
		return this->Identifier;
	}

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
	std::string Identifier;
	std::string Name;
	QColor Color; //color used to identify the province in the province map
	LandedTitle *County = nullptr;
	const ::Culture *Culture = nullptr;
	const ::Religion *Religion = nullptr;
	Holding *CapitalHolding = nullptr;
	std::vector<Holding *> Holdings;
	int MaxHoldings = 1;
};
