#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QDateTime>
#include <QVariant>

#include <string>
#include <vector>

namespace metternich {

class commodity;
class culture;
class Dynasty;
class gsml_property;
class holding;
class LandedTitle;
class phenotype;
class religion;
class Trait;

class Character : public numeric_data_entry, public DataType<Character, int>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring WRITE set_name_qstring NOTIFY name_changed)
	Q_PROPERTY(QString full_name READ GetFullNameQString NOTIFY FullNameChanged)
	Q_PROPERTY(QString titled_name READ GetTitledNameQString NOTIFY TitledNameChanged)
	Q_PROPERTY(bool female MEMBER Female READ IsFemale)
	Q_PROPERTY(metternich::Dynasty* dynasty READ GetDynasty WRITE SetDynasty NOTIFY DynastyChanged)
	Q_PROPERTY(metternich::culture* culture MEMBER culture READ get_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion MEMBER religion READ get_religion NOTIFY religion_changed)
	Q_PROPERTY(metternich::phenotype* phenotype MEMBER phenotype READ get_phenotype)
	Q_PROPERTY(metternich::LandedTitle* primary_title READ GetPrimaryTitle WRITE SetPrimaryTitle NOTIFY PrimaryTitleChanged)
	Q_PROPERTY(metternich::Character* father READ GetFather WRITE SetFather)
	Q_PROPERTY(metternich::Character* mother READ GetMother WRITE SetMother)
	Q_PROPERTY(metternich::Character* spouse READ GetSpouse WRITE SetSpouse)
	Q_PROPERTY(metternich::Character* liege READ GetLiege WRITE SetLiege NOTIFY LiegeChanged)
	Q_PROPERTY(metternich::Character* employer READ GetLiege WRITE SetLiege NOTIFY LiegeChanged)
	Q_PROPERTY(QVariantList traits READ GetTraitsQVariantList)
	Q_PROPERTY(int wealth READ GetWealth WRITE SetWealth NOTIFY WealthChanged)

public:
	static constexpr const char *ClassIdentifier = "character";
	static constexpr const char *DatabaseFolder = "characters";
	static constexpr bool HistoryOnly = true;

	static void Remove(Character *character)
	{
		if (character->IsAlive()) {
			Character::LivingCharacters.erase(std::remove(Character::LivingCharacters.begin(), Character::LivingCharacters.end(), character), Character::LivingCharacters.end());
		}

		DataType<Character, int>::Remove(character);
	}

	static const std::vector<Character *> &GetAllLiving()
	{
		return Character::LivingCharacters;
	}

	static Character *Generate(culture *culture, religion *religion, phenotype *phenotype = nullptr);

private:
	static inline std::vector<Character *> LivingCharacters;

public:
	Character(const int identifier) : numeric_data_entry(identifier)
	{
		connect(this, &Character::name_changed, this, &Character::FullNameChanged);
		connect(this, &Character::name_changed, this, &Character::TitledNameChanged);
		connect(this, &Character::DynastyChanged, this, &Character::FullNameChanged);
		connect(this, &Character::PrimaryTitleChanged, this, &Character::TitledNameChanged);

		Character::LivingCharacters.push_back(this);
	}

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

	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date) override;
	virtual void initialize_history() override;

	virtual void check() const override
	{
		if (this->get_name().empty()) {
			throw std::runtime_error("Character \"" + std::to_string(this->get_identifier()) + "\" has no name.");
		}

		if (this->get_culture() == nullptr) {
			throw std::runtime_error("Character \"" + std::to_string(this->get_identifier()) + "\" has no culture.");
		}

		if (this->get_religion() == nullptr) {
			throw std::runtime_error("Character \"" + std::to_string(this->get_identifier()) + "\" has no religion.");
		}

		if (this->get_phenotype() == nullptr) {
			throw std::runtime_error("Character \"" + std::to_string(this->get_identifier()) + "\" has no phenotype.");
		}
	}

	void DoMonth()
	{
	}

	virtual std::string get_name() const override
	{
		return this->name;
	}

	void set_name_qstring(const QString &name)
	{
		this->name = name.toStdString();
	}

	std::string GetFullName() const;

	QString GetFullNameQString() const
	{
		return QString::fromStdString(this->GetFullName());
	}

	std::string GetTitledName() const;

	QString GetTitledNameQString() const
	{
		return QString::fromStdString(this->GetTitledName());
	}

	bool IsAlive() const
	{
		return this->Alive;
	}

	void SetAlive(const bool alive)
	{
		if (alive == this->Alive) {
			return;
		}

		this->Alive = alive;
		if (this->Alive) {
			Character::LivingCharacters.push_back(this);
		} else {
			Character::LivingCharacters.erase(std::remove(Character::LivingCharacters.begin(), Character::LivingCharacters.end(), this), Character::LivingCharacters.end());
		}
		emit AliveChanged();
	}

	bool IsFemale() const
	{
		return this->Female;
	}

	metternich::Dynasty *GetDynasty() const
	{
		return this->Dynasty;
	}

	void SetDynasty(Dynasty *dynasty)
	{
		if (dynasty == this->GetDynasty()) {
			return;
		}

		this->Dynasty = dynasty;

		emit DynastyChanged();
	}

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	metternich::religion *get_religion() const
	{
		return this->religion;
	}

	metternich::phenotype *get_phenotype() const
	{
		return this->phenotype;
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
		emit LiegeChanged();
	}
	Character *GetTopLiege() const
	{
		if (this->GetLiege() != nullptr) {
			return this->GetLiege()->GetTopLiege();
		}

		return const_cast<Character *>(this);
	}

	bool IsAnyLiegeOf(const Character *character) const
	{
		if (character->GetLiege() == nullptr) {
			return false;
		} else if (this == character->GetLiege()) {
			return true;
		}

		return this->IsAnyLiegeOf(character->GetLiege());
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

	Q_INVOKABLE void add_trait(Trait *trait)
	{
		this->Traits.push_back(trait);
	}

	Q_INVOKABLE void remove_trait(Trait *trait)
	{
		this->Traits.erase(std::remove(this->Traits.begin(), this->Traits.end(), trait), this->Traits.end());
	}

	int GetWealth() const
	{
		return this->Wealth;
	}

	void SetWealth(const int wealth)
	{
		if (this->Wealth == wealth) {
			return;
		}

		this->Wealth = wealth;
		emit WealthChanged();
	}

	void ChangeWealth(const int change)
	{
		this->SetWealth(this->GetWealth() + change);
	}

	int get_stored_commodity(const commodity *commodity) const
	{
		auto find_iterator = this->stored_commodities.find(commodity);
		if (find_iterator == this->stored_commodities.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	bool can_build_in_holding(const holding *holding);
	Q_INVOKABLE bool can_build_in_holding(const QVariant &holding_variant);

signals:
	void name_changed();
	void FullNameChanged();
	void TitledNameChanged();
	void AliveChanged();
	void DynastyChanged();
	void culture_changed();
	void religion_changed();
	void PrimaryTitleChanged();
	void LiegeChanged();
	void WealthChanged();

private:
	std::string name;
	bool Alive = true;
	bool Female = false;
	metternich::Dynasty *Dynasty = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	metternich::phenotype *phenotype = nullptr;
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
	int Wealth = 0;
	std::map<const commodity *, int> stored_commodities; //the amount of each commodity stored by the character
};

}
