#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QDateTime>
#include <QVariant>

#include <string>
#include <vector>

namespace Metternich {

class Culture;
class Dynasty;
class GSMLProperty;
class LandedTitle;
class Religion;
class Trait;

class Character : public NumericDataEntry, public DataType<Character, int>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString WRITE SetNameQString)
	Q_PROPERTY(bool female MEMBER Female READ IsFemale)
	Q_PROPERTY(Metternich::Dynasty* dynasty MEMBER Dynasty READ GetDynasty NOTIFY DynastyChanged)
	Q_PROPERTY(Metternich::Culture* culture MEMBER Culture READ GetCulture NOTIFY CultureChanged)
	Q_PROPERTY(Metternich::Religion* religion MEMBER Religion READ GetReligion NOTIFY ReligionChanged)
	Q_PROPERTY(Metternich::LandedTitle* primary_title READ GetPrimaryTitle WRITE SetPrimaryTitle NOTIFY PrimaryTitleChanged)
	Q_PROPERTY(Metternich::Character* father READ GetFather WRITE SetFather)
	Q_PROPERTY(Metternich::Character* mother READ GetMother WRITE SetMother)
	Q_PROPERTY(Metternich::Character* spouse READ GetSpouse WRITE SetSpouse)
	Q_PROPERTY(Metternich::Character* liege READ GetLiege WRITE SetLiege)
	Q_PROPERTY(Metternich::Character* employer READ GetLiege WRITE SetLiege)
	Q_PROPERTY(QVariantList traits READ GetTraitsQVariantList)

public:
	static constexpr const char *ClassIdentifier = "character";
	static constexpr const char *DatabaseFolder = "characters";
	static constexpr bool HistoryOnly = true;

	static Character *Generate(Culture *culture, Religion *religion);

	Character(const int identifier) : NumericDataEntry(identifier) {}

	virtual ~Character() override
	{
		//remove references from other characters to his one; necessary since this character could be purged e.g. if it was born after the start date
		if (this->GetFather() != nullptr) {
			this->GetFather()->Children.erase(std::remove(this->GetFather()->Children.begin(), this->GetFather()->Children.end(), this), this->GetFather()->Children.end());
		}

		if (this->GetMother() != nullptr) {
			this->GetMother()->Children.erase(std::remove(this->GetMother()->Children.begin(), this->GetMother()->Children.end(), this), this->GetMother()->Children.end());
		}

		if (this->GetSpouse() != nullptr) {
			this->GetSpouse()->Spouse = nullptr;
		}

		for (Character *child : this->Children) {
			if (this->IsFemale()) {
				child->Mother = nullptr;
			} else {
				child->Father = nullptr;
			}
		}

		if (this->GetLiege() != nullptr) {
			this->GetLiege()->Vassals.erase(std::remove(this->GetLiege()->Vassals.begin(), this->GetLiege()->Vassals.end(), this), this->GetLiege()->Vassals.end());
		}

		for (Character *vassal : this->Vassals) {
			vassal->Liege = nullptr;
		}
	}

	virtual void ProcessGSMLHistoryProperty(const GSMLProperty &property, const QDateTime &date) override;
	virtual void Initialize() override;

	virtual void Check() const override
	{
		if (this->GetName().empty()) {
			throw std::runtime_error("Character \"" + std::to_string(this->GetIdentifier()) + "\" has no name.");
		}

		if (this->GetCulture() == nullptr) {
			throw std::runtime_error("Character \"" + std::to_string(this->GetIdentifier()) + "\" has no culture.");
		}

		if (this->GetReligion() == nullptr) {
			throw std::runtime_error("Character \"" + std::to_string(this->GetIdentifier()) + "\" has no religion.");
		}
	}

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

	Dynasty *GetDynasty() const
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

	Character *GetFather() const
	{
		return this->Father;
	}

	void SetFather(Character *father)
	{
		if (this->GetFather() == father) {
			return;
		}

		if (this->GetFather() != nullptr) {
			this->GetFather()->Children.erase(std::remove(this->GetFather()->Children.begin(), this->GetFather()->Children.end(), this), this->GetFather()->Children.end());
		}

		this->Father = father;
		father->Children.push_back(this);
	}

	Character *GetMother() const
	{
		return this->Mother;
	}

	void SetMother(Character *mother)
	{
		if (this->GetMother() == mother) {
			return;
		}

		if (this->GetMother() != nullptr) {
			this->GetMother()->Children.erase(std::remove(this->GetMother()->Children.begin(), this->GetMother()->Children.end(), this), this->GetMother()->Children.end());
		}

		this->Mother = mother;
		mother->Children.push_back(this);
	}

	Character *GetSpouse() const
	{
		return this->Spouse;
	}

	void SetSpouse(Character *spouse)
	{
		if (this->GetSpouse() == spouse) {
			return;
		}

		if (this->GetSpouse() != nullptr) {
			this->GetSpouse()->Spouse = nullptr;
		}

		this->Spouse = spouse;
		spouse->Spouse = this;
	}

	const QDateTime &GetBirthDate()
	{
		return this->BirthDate;
	}

	const QDateTime &GetDeathDate()
	{
		return this->DeathDate;
	}

	Character *GetLiege() const
	{
		return this->Liege;
	}

	void SetLiege(Character *liege)
	{
		if (this->GetLiege() == liege) {
			return;
		}

		if (this->GetLiege() != nullptr) {
			this->GetLiege()->Vassals.erase(std::remove(this->GetLiege()->Vassals.begin(), this->GetLiege()->Vassals.end(), this), this->GetLiege()->Vassals.end());
		}

		this->Liege = liege;
		liege->Vassals.push_back(this);
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

	const std::vector<Trait *> &GetTraits() const
	{
		return this->Traits;
	}

	QVariantList GetTraitsQVariantList() const;

	Q_INVOKABLE void AddTrait(Trait *trait)
	{
		this->Traits.push_back(trait);
	}

	Q_INVOKABLE void RemoveTrait(Trait *trait)
	{
		this->Traits.erase(std::remove(this->Traits.begin(), this->Traits.end(), trait), this->Traits.end());
	}

signals:
	void DynastyChanged();
	void CultureChanged();
	void ReligionChanged();
	void PrimaryTitleChanged();

private:
	std::string Name;
	bool Alive = true;
	bool Female = false;
	Dynasty *Dynasty = nullptr;
	Culture *Culture = nullptr;
	Religion *Religion = nullptr;
	LandedTitle *PrimaryTitle = nullptr;
	std::vector<LandedTitle *> LandedTitles;
	Character *Father = nullptr;
	Character *Mother = nullptr;
	std::vector<Character *> Children;
	Character *Spouse = nullptr;
	QDateTime BirthDate;
	QDateTime DeathDate;
	Character *Liege = nullptr;
	std::vector<Character *> Vassals;
	std::vector<Trait *> Traits;
};

}
