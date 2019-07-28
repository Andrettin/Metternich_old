#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QObject>
#include <QRect>

#include <map>
#include <set>
#include <string>
#include <vector>

class CSVData;
class Culture;
class Holding;
class LandedTitle;
class Religion;

class Province : public QObject, public DataEntry<>, public DataType<Province>
{
	Q_OBJECT

	Q_PROPERTY(QString identifier READ GetIdentifierQString CONSTANT)
	Q_PROPERTY(QColor color READ GetColor CONSTANT)
	Q_PROPERTY(QRect rect READ GetRect CONSTANT)
	Q_PROPERTY(QImage image READ GetImage NOTIFY ImageChanged)
	Q_PROPERTY(bool selected READ IsSelected WRITE SetSelected NOTIFY SelectedChanged)

public:
	Province(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "province";
	static constexpr const char *DatabaseFolder = "provinces";
	static constexpr const char *Prefix = "p_";

	static Province *Add(const std::string &identifier);
	static Province *GetByRGB(const QRgb &rgb);

private:
	static inline std::map<QRgb, Province *> InstancesByRGB;
	static inline Province *SelectedProvince = nullptr;

public:
	virtual bool ProcessGSMLProperty(const GSMLProperty &property) override;
	virtual bool ProcessGSMLScope(const GSMLData &scope) override;

	QString GetIdentifierQString() const
	{
		return QString::fromStdString(this->GetIdentifier());
	}

	LandedTitle *GetCounty() const
	{
		return this->County;
	}

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

	void SetSelected(const bool selected);

	bool IsSelected() const
	{
		return this->Selected;
	}

signals:
	void ImageChanged();
	void SelectedChanged();

private:
	LandedTitle *County = nullptr;
	QColor Color; //color used to identify the province in the province map
	QRect Rect; //the rectangle that the province occupies
	QImage Image; //the province's image to be drawn on-screen
	const ::Culture *Culture = nullptr;
	const ::Religion *Religion = nullptr;
	Holding *CapitalHolding = nullptr;
	std::vector<Holding *> Holdings;
	int MaxHoldings = 1;
	bool Selected = false;
};
