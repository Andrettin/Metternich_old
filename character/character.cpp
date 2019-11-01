#include "character/character.h"

#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_property.h"
#include "game/game.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "landed_title/landed_title_tier.h"
#include "random.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/parse_util.h"

#include <QVariant>

namespace metternich {

/**
**	@brief	Generate a character
**
**	@param	culture		The culture for the generated character
**	@param	religion	The religion for the generated character
**	@param	phenotype	The phenotype for the generated character; if none is given then the default phenotype for the culture is used instead
*/
character *character::generate(metternich::culture *culture, metternich::religion *religion, metternich::phenotype *phenotype)
{
	const int identifier = character::generate_numeric_identifier();
	character *character = character::add(identifier);
	character->culture = culture;
	character->religion = religion;
	if (phenotype != nullptr) {
		character->phenotype = phenotype;
	}
	//generate the character's birth date to be between 60 and 20 years before the current date
	const QDateTime &current_date = game::get()->get_current_date();
	character->birth_date = current_date.addDays(random::generate_in_range(-60 * 365, -20 * 365));
	character->initialize_history(); //generates a name and sets the phenotype if none was given
	return character;
}

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
**	@param	date		The date of the property change
*/
void character::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	if (property.get_key() == "birth") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (util::string_to_bool(property.get_value())) {
			this->birth_date = date;
			this->alive = true;
		}
	} else if (property.get_key() == "death") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (util::string_to_bool(property.get_value())) {
			this->death_date = date;
			this->alive = false;
		}
	} else {
		this->process_gsml_property(property);
	}
}

/**
**	@brief	Initialize the character's history
*/
void character::initialize_history()
{
	if (this->get_phenotype() == nullptr) {
		if (this->get_culture()->get_default_phenotype() != nullptr) {
			this->phenotype = this->get_culture()->get_default_phenotype();
		} else if (this->get_culture()->get_culture_group()->get_default_phenotype() != nullptr) {
			this->phenotype = this->get_culture()->get_culture_group()->get_default_phenotype();
		}
	}

	if (this->name.empty() && this->get_culture() != nullptr) {
		if (this->is_female()) {
			this->name = this->get_culture()->generate_female_name();
		} else {
			this->name = this->get_culture()->generate_male_name();
		}
	}
}

/**
**	@brief	Get the character's full name
**
**	@return	The character's full name
*/
std::string character::get_full_name() const
{
	std::string full_name = this->name;
	if (this->get_dynasty() != nullptr) {
		full_name += " " + this->get_dynasty()->get_name();
	}
	return full_name;
}

/**
**	@brief	Get the character's titled name
**
**	@return	The character's titled name
*/
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

QVariantList character::get_traits_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_traits());
}

/**
**	@brief	Get whether the character can build in a holding
**
**	@param	holding	The holding
*/
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
