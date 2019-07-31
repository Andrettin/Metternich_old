#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QDateTime>

#include <string>
#include <vector>

namespace Metternich {

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;

class Character : public NumericDataEntry, public DataType<Character, int>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString WRITE SetNameQString)
	Q_PROPERTY(bool female MEMBER Female READ IsFemale)
	Q_PROPERTY(Metternich::Culture* culture MEMBER Culture READ GetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion MEMBER Religion READ GetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(Metternich::LandedTitle* primary_title READ GetPrimaryTitle WRITE SetPrimaryTitle NOTIFY PrimaryTitleChanged)

public:
	Character(const int identifier) : NumericDataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "character";
	static constexpr const char *DatabaseFolder = "characters";

	virtual void ProcessGSMLHistoryProperty(const GSMLProperty &property, const QDateTime &date) override;

	virtual std::string GetName() const override
	{
		return this->Name;
	}

	void SetNameQString(const QString &name)
	{
		this->Name = name.toStdString();
	}

	bool IsAlive() const
	{
		return this->Alive;
	}

	bool IsFemale() const
	{
		return this->Female;
	}

	const Dynasty *GetDynasty() const
	{
		return this->Dynasty;
	}

	Culture *GetCulture() const
	{
		return this->Culture;
	}

	Religion *GetReligion() const
	{
		return this->Religion;
	}

	LandedTitle *GetPrimaryTitle() const
	{
		return this->PrimaryTitle;
	}

	void SetPrimaryTitle(LandedTitle *title)
	{
		if (title == this->GetPrimaryTitle()) {
			return;
		}

		this->PrimaryTitle = title;

		emit PrimaryTitleChanged();
	}

	void ChoosePrimaryTitle();

	const std::vector<LandedTitle *> &GetLandedTitles() const
	{
		return this->LandedTitles;
	}

	void AddLandedTitle(LandedTitle *title);
	void RemoveLandedTitle(LandedTitle *title);

	Character *GetLiege() const
	{
		return this->Liege;
	}

	Character *GetTopLiege() const
	{
		if (this->GetLiege() != nullptr) {
			return this->GetLiege()->GetTopLiege();
		}

		return const_cast<Character *>(this);
	}

	LandedTitle *GetRealm() const
	{
		Character *top_liege = this->GetTopLiege();
		return top_liege->GetPrimaryTitle();
	}

signals:
	void CultureChanged();
	void ReligionChanged();
	void PrimaryTitleChanged();

private:
	std::string Name;
	bool Alive = false;
	bool Female = false;
	Dynasty *Dynasty = nullptr;
	Culture *Culture = nullptr;
	Religion *Religion = nullptr;
	LandedTitle *PrimaryTitle = nullptr;
	std::vector<LandedTitle *> LandedTitles;
	Character *Father = nullptr;
	Character *Mother = nullptr;
	std::vector<Character *> Children;
	QDateTime BirthDate;
	QDateTime DeathDate;
	Character *Liege = nullptr;
};

}
