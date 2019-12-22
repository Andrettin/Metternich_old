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
class dynasty;
class gsml_property;
class holding;
class landed_title;
class phenotype;
class religion;
class trait;

class character : public numeric_data_entry, public data_type<character, int>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring WRITE set_name_qstring NOTIFY name_changed)
	Q_PROPERTY(QString full_name READ get_full_name_qstring NOTIFY full_name_changed)
	Q_PROPERTY(QString titled_name READ get_titled_name_qstring NOTIFY titled_name_changed)
	Q_PROPERTY(bool female MEMBER female READ is_female)
	Q_PROPERTY(metternich::dynasty* dynasty READ get_dynasty WRITE set_dynasty NOTIFY dynasty_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion MEMBER religion READ get_religion NOTIFY religion_changed)
	Q_PROPERTY(metternich::phenotype* phenotype MEMBER phenotype READ get_phenotype)
	Q_PROPERTY(metternich::landed_title* primary_title READ get_primary_title WRITE set_primary_title NOTIFY primary_title_changed)
	Q_PROPERTY(metternich::character* father READ get_father WRITE set_father)
	Q_PROPERTY(metternich::character* mother READ get_mother WRITE set_mother)
	Q_PROPERTY(metternich::character* spouse READ get_spouse WRITE set_spouse)
	Q_PROPERTY(metternich::character* liege READ get_liege WRITE set_liege NOTIFY liege_changed)
	Q_PROPERTY(metternich::character* employer READ get_liege WRITE set_liege NOTIFY liege_changed)
	Q_PROPERTY(QVariantList traits READ get_traits_qvariant_list NOTIFY traits_changed)
	Q_PROPERTY(int wealth READ get_wealth WRITE set_wealth NOTIFY wealth_changed)

public:
	static constexpr const char *class_identifier = "character";
	static constexpr const char *database_folder = "characters";
	static constexpr bool history_only = true;

	static std::set<std::string> get_database_dependencies();

	static void remove(character *character)
	{
		if (character->is_alive()) {
			character::living_characters.erase(std::remove(character::living_characters.begin(), character::living_characters.end(), character), character::living_characters.end());
		}

		data_type<metternich::character, int>::remove(character);
	}

	static const std::vector<character *> &get_all_living()
	{
		return character::living_characters;
	}

	static character *generate(culture *culture, religion *religion, phenotype *phenotype = nullptr);

private:
	static inline std::vector<character *> living_characters;

public:
	character(const int identifier) : numeric_data_entry(identifier)
	{
		connect(this, &character::name_changed, this, &character::full_name_changed);
		connect(this, &character::name_changed, this, &character::titled_name_changed);
		connect(this, &character::dynasty_changed, this, &character::full_name_changed);
		connect(this, &character::primary_title_changed, this, &character::titled_name_changed);

		character::living_characters.push_back(this);
	}

	virtual ~character() override
	{
		//remove references from other characters to his one; necessary since this character could be purged e.g. if it was born after the start date
		if (this->get_father() != nullptr) {
			this->get_father()->children.erase(std::remove(this->get_father()->children.begin(), this->get_father()->children.end(), this), this->get_father()->children.end());
		}

		if (this->get_mother() != nullptr) {
			this->get_mother()->children.erase(std::remove(this->get_mother()->children.begin(), this->get_mother()->children.end(), this), this->get_mother()->children.end());
		}

		if (this->get_spouse() != nullptr) {
			this->get_spouse()->spouse = nullptr;
		}

		for (character *child : this->children) {
			if (this->is_female()) {
				child->mother = nullptr;
			} else {
				child->father = nullptr;
			}
		}

		if (this->get_liege() != nullptr) {
			this->get_liege()->vassals.erase(std::remove(this->get_liege()->vassals.begin(), this->get_liege()->vassals.end(), this), this->get_liege()->vassals.end());
		}

		for (character *vassal : this->vassals) {
			vassal->liege = nullptr;
		}
	}

	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date) override;
	virtual void initialize_history() override;

	virtual void check_history() const override
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

	void do_month();

	virtual std::string get_name() const override
	{
		return this->name;
	}

	void set_name(const std::string &name);

	void set_name_qstring(const QString &name)
	{
		this->set_name(name.toStdString());
	}

	std::string get_full_name() const;

	QString get_full_name_qstring() const
	{
		return QString::fromStdString(this->get_full_name());
	}

	std::string get_titled_name() const;

	QString get_titled_name_qstring() const
	{
		return QString::fromStdString(this->get_titled_name());
	}

	bool is_alive() const
	{
		return this->alive;
	}

	void set_alive(const bool alive)
	{
		if (alive == this->alive) {
			return;
		}

		this->alive = alive;
		if (this->alive) {
			character::living_characters.push_back(this);
		} else {
			character::living_characters.erase(std::remove(character::living_characters.begin(), character::living_characters.end(), this), character::living_characters.end());
		}
		emit alive_changed();
	}

	bool is_female() const
	{
		return this->female;
	}

	metternich::dynasty *get_dynasty() const
	{
		return this->dynasty;
	}

	void set_dynasty(dynasty *dynasty)
	{
		if (dynasty == this->get_dynasty()) {
			return;
		}

		this->dynasty = dynasty;

		emit dynasty_changed();
	}

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture);

	metternich::religion *get_religion() const
	{
		return this->religion;
	}

	metternich::phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	landed_title *get_primary_title() const
	{
		return this->primary_title;
	}

	void set_primary_title(landed_title *title)
	{
		if (title == this->get_primary_title()) {
			return;
		}

		this->primary_title = title;

		emit primary_title_changed();
	}

	void choose_primary_title();

	const std::vector<landed_title *> &get_landed_titles() const
	{
		return this->landed_titles;
	}

	bool has_landed_title(const landed_title *title) const
	{
		return std::find(this->landed_titles.begin(), this->landed_titles.end(), title) != this->landed_titles.end();
	}

	void add_landed_title(landed_title *title);
	void remove_landed_title(landed_title *title);

	character *get_father() const
	{
		return this->father;
	}

	void set_father(character *father)
	{
		if (this->get_father() == father) {
			return;
		}

		if (this->get_father() != nullptr) {
			this->get_father()->children.erase(std::remove(this->get_father()->children.begin(), this->get_father()->children.end(), this), this->get_father()->children.end());
		}

		this->father = father;
		father->children.push_back(this);
	}

	character *get_mother() const
	{
		return this->mother;
	}

	void set_mother(character *mother)
	{
		if (this->get_mother() == mother) {
			return;
		}

		if (this->get_mother() != nullptr) {
			this->get_mother()->children.erase(std::remove(this->get_mother()->children.begin(), this->get_mother()->children.end(), this), this->get_mother()->children.end());
		}

		this->mother = mother;
		mother->children.push_back(this);
	}

	character *get_spouse() const
	{
		return this->spouse;
	}

	void set_spouse(character *spouse)
	{
		if (this->get_spouse() == spouse) {
			return;
		}

		if (this->get_spouse() != nullptr) {
			this->get_spouse()->spouse = nullptr;
		}

		this->spouse = spouse;
		spouse->spouse = this;
	}

	const QDateTime &get_birth_date()
	{
		return this->birth_date;
	}

	const QDateTime &get_death_date()
	{
		return this->death_date;
	}

	character *get_liege() const
	{
		return this->liege;
	}

	void set_liege(character *liege)
	{
		if (this->get_liege() == liege) {
			return;
		}

		if (this->get_liege() != nullptr) {
			this->get_liege()->vassals.erase(std::remove(this->get_liege()->vassals.begin(), this->get_liege()->vassals.end(), this), this->get_liege()->vassals.end());
		}

		this->liege = liege;
		liege->vassals.push_back(this);
		emit liege_changed();
	}
	character *get_top_liege() const
	{
		if (this->get_liege() != nullptr) {
			return this->get_liege()->get_top_liege();
		}

		return const_cast<character *>(this);
	}

	bool is_any_liege_of(const character *character) const
	{
		if (character->get_liege() == nullptr) {
			return false;
		} else if (this == character->get_liege()) {
			return true;
		}

		return this->is_any_liege_of(character->get_liege());
	}

	landed_title *get_realm() const
	{
		character *top_liege = this->get_top_liege();
		return top_liege->get_primary_title();
	}

	const std::vector<trait *> &get_traits() const
	{
		return this->traits;
	}

	QVariantList get_traits_qvariant_list() const;

	Q_INVOKABLE void add_trait(trait *trait)
	{
		this->traits.push_back(trait);
		emit traits_changed();
	}

	Q_INVOKABLE void remove_trait(trait *trait)
	{
		this->traits.erase(std::remove(this->traits.begin(), this->traits.end(), trait), this->traits.end());
		emit traits_changed();
	}

	bool has_trait(const trait *trait) const
	{
		return std::find(this->traits.begin(), this->traits.end(), trait) != this->traits.end();
	}

	bool has_personality_trait() const;
	void generate_personality_trait();

	int get_wealth() const
	{
		return this->wealth;
	}

	void set_wealth(const int wealth)
	{
		if (this->wealth == wealth) {
			return;
		}

		this->wealth = wealth;
		emit wealth_changed();
	}

	void change_wealth(const int change)
	{
		this->set_wealth(this->get_wealth() + change);
	}

	int get_stored_commodity(const commodity *commodity) const
	{
		auto find_iterator = this->stored_commodities.find(commodity);
		if (find_iterator == this->stored_commodities.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	bool is_ai() const;

	bool can_build_in_holding(const holding *holding);
	Q_INVOKABLE bool can_build_in_holding(const QVariant &holding_variant);

signals:
	void name_changed();
	void full_name_changed();
	void titled_name_changed();
	void alive_changed();
	void dynasty_changed();
	void culture_changed();
	void religion_changed();
	void primary_title_changed();
	void liege_changed();
	void wealth_changed();
	void traits_changed();

private:
	std::string name;
	bool alive = true;
	bool female = false;
	metternich::dynasty *dynasty = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	metternich::phenotype *phenotype = nullptr;
	landed_title *primary_title = nullptr;
	std::vector<landed_title *> landed_titles;
	character *father = nullptr;
	character *mother = nullptr;
	std::vector<character *> children;
	character *spouse = nullptr;
	QDateTime birth_date;
	QDateTime death_date;
	character *liege = nullptr;
	std::vector<character *> vassals;
	std::vector<trait *> traits;
	int wealth = 0;
	std::map<const commodity *, int> stored_commodities; //the amount of each commodity stored by the character
};

}
