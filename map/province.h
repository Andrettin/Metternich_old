#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QImage>
#include <QObject>
#include <QRect>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Metternich {

class CSVData;
class Culture;
class Holding;
class HoldingType;
class LandedTitle;
class Religion;

class Province : public DataEntry, public DataType<Province>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::LandedTitle* county READ GetCounty WRITE SetCounty NOTIFY CountyChanged)
	Q_PROPERTY(QColor color READ GetColor CONSTANT)
	Q_PROPERTY(QRect rect READ GetRect CONSTANT)
	Q_PROPERTY(QImage image READ GetImage NOTIFY ImageChanged)
	Q_PROPERTY(Metternich::Culture* culture MEMBER Culture READ GetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion MEMBER Religion READ GetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(QVariantList holdings READ GetHoldingsQVariantList NOTIFY HoldingsChanged)
	Q_PROPERTY(Metternich::Holding* capital_holding READ GetCapitalHolding WRITE SetCapitalHolding NOTIFY CapitalHoldingChanged)
	Q_PROPERTY(bool selected READ IsSelected WRITE SetSelected NOTIFY SelectedChanged)

public:
	Province(const std::string &identifier);
	virtual ~Province() override;

	static constexpr const char *ClassIdentifier = "province";
	static constexpr const char *DatabaseFolder = "provinces";
	static constexpr const char *Prefix = "p_";

	static Province *Add(const std::string &identifier);
	static Province *GetByRGB(const QRgb &rgb);

	static Province *GetSelectedProvince()
	{
		return Province::SelectedProvince;
	}

private:
	static inline std::map<QRgb, Province *> InstancesByRGB;
	static inline Province *SelectedProvince = nullptr;

public:
	virtual void ProcessGSMLProperty(const GSMLProperty &property) override;
	virtual void ProcessGSMLScope(const GSMLData &scope) override;
	virtual void Check() const override;

	virtual std::string GetName() const override;

	LandedTitle *GetCounty() const
	{
		return this->County;
	}

	void SetCounty(LandedTitle *county);

	const QColor &GetColor() const
	{
		return this->Color;
	}

	const QRect &GetRect() const
	{
		return this->Rect;
	}

	void CreateImage(const std::set<int> &pixel_indexes);
	void UpdateImage();

	const QImage &GetImage() const
	{
		return this->Image;
	}

	Culture *GetCulture() const
	{
		return this->Culture;
	}

	Religion *GetReligion() const
	{
		return this->Religion;
	}

	int GetMaxSettlementHoldings() const
	{
		return this->MaxSettlementHoldings;
	}

	const std::vector<Holding *> &GetHoldings() const
	{
		return this->Holdings;
	}

	QVariantList GetHoldingsQVariantList() const;
	Holding *GetHolding(LandedTitle *barony) const;
	void CreateHolding(LandedTitle *barony, HoldingType *type);
	void DestroyHolding(LandedTitle *barony);

	Holding *GetCapitalHolding() const
	{
		return this->CapitalHolding;
	}

	void SetCapitalHolding(Holding *holding)
	{
		if (holding == this->GetCapitalHolding()) {
			return;
		}

		this->CapitalHolding = holding;
		emit CapitalHoldingChanged();
	}

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify = true);

signals:
	void CountyChanged();
	void ImageChanged();
	void CultureChanged();
	void ReligionChanged();
	void HoldingsChanged();
	void CapitalHoldingChanged();
	void SelectedChanged();

private:
	LandedTitle *County = nullptr;
	QColor Color; //color used to identify the province in the province map
	QRect Rect; //the rectangle that the province occupies
	QImage Image; //the province's image to be drawn on-screen
	Culture *Culture = nullptr;
	Religion *Religion = nullptr;
	std::vector<Holding *> Holdings;
	std::map<LandedTitle *, std::unique_ptr<Holding>> HoldingsByBarony; //the province's holdings, mapped to their respective baronies
	Holding *CapitalHolding = nullptr;
	int MaxSettlementHoldings = 1;
	bool Selected = false;
};

}
