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
	Q_PROPERTY(Metternich::Culture* culture READ GetCulture WRITE SetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion READ GetReligion WRITE SetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(int size READ GetSize WRITE SetSize NOTIFY SizeChanged)
	Q_PROPERTY(Metternich::Holding* holding MEMBER Holding READ GetHolding NOTIFY HoldingChanged)

public:
	PopulationUnit(PopulationType *type, Holding *holding) : Type(type), Holding(holding) {}

	PopulationType *GetType() const
	{
		return this->Type;
	}

	Metternich::Culture *GetCulture() const
	{
		return this->Culture;
	}

	void SetCulture(Culture *culture)
	{
		if (culture == this->GetCulture()) {
			return;
		}

		this->Culture = culture;
		emit CultureChanged();
	}

	Metternich::Religion *GetReligion() const
	{
		return this->Religion;
	}

	void SetReligion(Religion *religion)
	{
		if (religion == this->GetReligion()) {
			return;
		}

		this->Religion = religion;
		emit ReligionChanged();
	}

	int GetSize() const
	{
		return this->Size;
	}

	void SetSize(const int size)
	{
		if (size == this->GetSize()) {
			return;
		}

		this->Size = size;
		emit SizeChanged();
	}

	void ChangeSize(const int change)
	{
		this->SetSize(this->GetSize() + change);
	}

	Metternich::Holding *GetHolding() const
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
