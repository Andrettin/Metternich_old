#pragma once

#include <QObject>

namespace Metternich {

class Culture;
class Holding;
class PopulationType;
class Religion;

class PopulationUnit : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Metternich::PopulationType* type MEMBER Type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::Culture* culture MEMBER Culture READ GetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion MEMBER Religion READ GetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(int size MEMBER Size READ GetSize NOTIFY SizeChanged)
	Q_PROPERTY(Metternich::Holding* holding MEMBER Holding READ GetHolding NOTIFY HoldingChanged)

public:
	PopulationType *GetType() const
	{
		return this->Type;
	}

	Culture *GetCulture() const
	{
		return this->Culture;
	}

	Religion *GetReligion() const
	{
		return this->Religion;
	}

	int GetSize() const
	{
		return this->Size;
	}

	Holding *GetHolding() const
	{
		return this->Holding;
	}

signals:
	void TypeChanged();
	void CultureChanged();
	void ReligionChanged();
	void SizeChanged();
	void HoldingChanged();

private:
	PopulationType *Type = nullptr;
	Metternich::Culture *Culture = nullptr;
	Metternich::Religion *Religion = nullptr;
	int Size = 0; //the size of the population unit, in number of individuals
	Metternich::Holding *Holding = nullptr; //the settlement holding where this population unit lives
};

}
