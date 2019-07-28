#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QRect>

#include <map>
#include <string>
#include <vector>

class CSVData;
class Culture;
class Holding;
class LandedTitle;
class Religion;

class Province : public DataEntry<>, public DataType<Province>
{
public:
	Province(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "province";
	static constexpr const char *DatabaseFolder = "provinces";
	static constexpr const char *Prefix = "p_";

	static Province *Add(const std::string &identifier);
	static Province *Get(const QRgb &rgb);

private:
	static inline std::map<QRgb, Province *> InstancesByRgb;

public:
	virtual bool ProcessGSMLProperty(const GSMLProperty &property) override;
	virtual bool ProcessGSMLScope(const GSMLData &scope) override;

	void SetRect(const QRect &rect)
	{
		this->Rect = rect;
	}

	const QRect &GetRect() const
	{
		return this->Rect;
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
	QRect Rect; //the rectangle that the province occupies
	QColor Color; //color used to identify the province in the province map
	LandedTitle *County = nullptr;
	const ::Culture *Culture = nullptr;
	const ::Religion *Religion = nullptr;
	Holding *CapitalHolding = nullptr;
	std::vector<Holding *> Holdings;
	int MaxHoldings = 1;
};
