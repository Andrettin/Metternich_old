#include "character/character.h"

#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "game/game.h"
#include "history/history.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "landed_title/landed_title_tier.h"
#include "random.h"
#include "script/event/character_event.h"
#include "util/container_util.h"
#include "util/string_util.h"

#include <QVariant>

namespace metternich {

std::set<std::string> character::get_database_dependencies()
{
	return {
		//so that random holders will have been generated when all characters have their history initialized
		landed_title::class_identifier
	};
}

character *character::generate(metternich::culture *culture, metternich::religion *religion, metternich::phenotype *phenotype)
{
	if (culture == nullptr) {
		throw std::runtime_error("Tried to generate a character with no culture.");
	}

	if (religion == nullptr) {
		throw std::runtime_error("Tried to generate a character with no religion.");
	}

	const std::string identifier = character::generate_identifier();
	character *character = character::add(identifier);
	character->culture = culture;
	character->religion = religion;
	if (phenotype != nullptr) {
		character->phenotype = phenotype;
	}

	//generate the character's birth date to be between 60 and 20 years before the current date
	const QDateTime &current_date = game::get()->get_current_date();
	character->birth_date = current_date.addDays(random::generate_in_range(-60 * 365, -20 * 365));
	if (!history::get()->is_loading()) { //if history is loading then this entry's history will already be initialized later on anyway
		character->initialize_history(); //generates a name and sets the phenotype if none was given
	}
	return character;
}

void character::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	if (property.get_key() == "birth") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (string::to_bool(property.get_value())) {
			this->birth_date = date;
			this->alive = true;
		}
	} else if (property.get_key() == "death") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (string::to_bool(property.get_value())) {
			this->death_date = date;
			this->alive = false;
		}
	} else {
		this->process_gsml_property(property);
	}
}

void character::initialize_history()
{
	if (history::get()->is_loading()) {
		if (this->get_culture() != nullptr && !this->name.empty()) {
			//add the character's name to its culture's name list
			if (this->is_female()) {
				this->get_culture()->add_female_name(this->name);
				this->get_culture()->get_culture_group()->add_female_name(this->name);
			} else {
				this->get_culture()->add_male_name(this->name);
				this->get_culture()->get_culture_group()->add_male_name(this->name);
			}
		}
	}

	if (this->get_culture() != nullptr) {
		if (this->get_phenotype() == nullptr) {
			this->phenotype = this->get_culture()->get_default_phenotype();
		}

		if (this->name.empty()) {
			if (this->is_female()) {
				this->name = this->get_culture()->generate_female_name();
			} else {
				this->name = this->get_culture()->generate_male_name();
			}
		}
	}

	if (!this->has_personality_trait()) {
		this->generate_personality_trait();
	}

	data_entry_base::initialize_history();
}

void character::do_month()
{
	//do character events
	for (character_event *event : character_event::get_all()) {
		if (event->check_conditions(this)) {
			event->do_event(this);
		}
	}
}

void character::set_name(const std::string &name)
{
	if (name == this->name) {
		return;
	}

	this->name = name;
	emit name_changed();
}

std::string character::get_full_name() const
{
	std::string full_name = this->name;
	if (this->get_dynasty() != nullptr) {
		full_name += " " + this->get_dynasty()->get_name();
	}
	return full_name;
}

std::string character::get_titled_name() const
{
	std::string titled_name;

	if (this->get_primary_title() != nullptr) {
		titled_name += this->get_primary_title()->get_holder_title_name() + " ";
	}

	titled_name += this->get_name();

	if (this->get_primary_title() != nullptr) {
		titled_name += " of " + this->get_primary_title()->get_name();
	}

	return titled_name;
}

void character::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	this->culture = culture;
	emit culture_changed();
}

void character::choose_primary_title()
{
	landed_title *best_title = nullptr;
	landed_title_tier best_title_tier = landed_title_tier::barony;

	for (landed_title *title : this->get_landed_titles()) {
		if (best_title == nullptr || title->get_tier() > best_title_tier) {
			best_title = title;
			best_title_tier = title->get_tier();
		}
	}

	this->set_primary_title(best_title);
}

void character::add_landed_title(landed_title *title)
{
	this->landed_titles.push_back(title);

	//if the new title belongs to a higher tier than the current primary title (or if there is no current primary title), change the character's primary title to the new title
	if (this->get_primary_title() == nullptr || this->get_primary_title()->get_tier() < title->get_tier()) {
		this->set_primary_title(title);
	}
}

void character::remove_landed_title(landed_title *title)
{
	this->landed_titles.erase(std::remove(this->landed_titles.begin(), this->landed_titles.end(), title), this->landed_titles.end());

	if (title == this->get_primary_title()) {
		this->choose_primary_title(); //needs to choose a new primary title, as the old one has been lost
	}
}

province *character::get_capital_province() const
{
	if (this->get_primary_title() != nullptr) {
		return this->get_primary_title()->get_capital_province();
	} else if (this->get_liege() != nullptr) {
		return this->get_liege()->get_capital_province();
	}

	throw std::runtime_error("Character \"" + this->get_identifier() + "\" has no capital province.");
}

QVariantList character::get_traits_qvariant_list() const
{
	return container::to_qvariant_list(this->get_traits());
}

bool character::has_personality_trait() const
{
	for (const trait *trait : this->get_traits()) {
		if (trait->is_personality()) {
			return true;
		}
	}

	return false;
}

void character::generate_personality_trait()
{
	std::vector<trait *> potential_traits;

	for (trait *trait : trait::get_personality_traits()) {
		potential_traits.push_back(trait);
	}

	if (potential_traits.empty()) {
		throw std::runtime_error("Could not generate personality trait for character \"" + this->get_identifier() + "\".");
	}

	trait *chosen_trait = potential_traits[random::generate(potential_traits.size())];
	this->add_trait(chosen_trait);
}

bool character::is_ai() const
{
	return game::get()->get_player_character() != this;
}

bool character::can_build_in_holding(const holding *holding)
{
	return holding->get_owner() == this || this->is_any_liege_of(holding->get_owner());
}

bool character::can_build_in_holding(const QVariant &holding_variant)
{
	QObject *holding_object = qvariant_cast<QObject *>(holding_variant);
	const holding *holding = static_cast<metternich::holding *>(holding_object);
	return this->can_build_in_holding(holding);
}

}
