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
Character *Character::generate(metternich::culture *culture, metternich::religion *religion, metternich::phenotype *phenotype)
{
	const int identifier = Character::generate_numeric_identifier();
	Character *character = Character::add(identifier);
	character->culture = culture;
	character->religion = religion;
	if (phenotype != nullptr) {
		character->phenotype = phenotype;
	}
	//generate the character's birth date to be between 60 and 20 years before the current date
	const QDateTime &current_date = Game::get()->GetCurrentDate();
	character->BirthDate = current_date.addDays(Random::generate_in_range(-60 * 365, -20 * 365));
	character->initialize_history(); //generates a name and sets the phenotype if none was given
	return character;
}

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
**	@param	date		The date of the property change
*/
void Character::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	if (property.get_key() == "birth") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (util::string_to_bool(property.get_value())) {
			this->BirthDate = date;
			this->Alive = true;
		}
	} else if (property.get_key() == "death") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (util::string_to_bool(property.get_value())) {
			this->DeathDate = date;
			this->Alive = false;
		}
	} else {
		this->process_gsml_property(property);
	}
}

/**
**	@brief	Initialize the character's history
*/
void Character::initialize_history()
{
	if (this->get_phenotype() == nullptr) {
		if (this->get_culture()->get_default_phenotype() != nullptr) {
			this->phenotype = this->get_culture()->get_default_phenotype();
		} else if (this->get_culture()->get_culture_group()->get_default_phenotype() != nullptr) {
			this->phenotype = this->get_culture()->get_culture_group()->get_default_phenotype();
		}
	}

	if (this->name.empty() && this->get_culture() != nullptr) {
		if (this->IsFemale()) {
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
std::string Character::GetFullName() const
{
	std::string full_name = this->name;
	if (this->GetDynasty() != nullptr) {
		full_name += " " + this->GetDynasty()->get_name();
	}
	return full_name;
}

/**
**	@brief	Get the character's titled name
**
**	@return	The character's titled name
*/
std::string Character::GetTitledName() const
{
	std::string titled_name;

	if (this->GetPrimaryTitle() != nullptr) {
		titled_name += this->GetPrimaryTitle()->GetHolderTitleName() + " ";
	}

	titled_name += this->get_name();

	if (this->GetPrimaryTitle() != nullptr) {
		titled_name += " of " + this->GetPrimaryTitle()->get_name();
	}

	return titled_name;
}

void Character::ChoosePrimaryTitle()
{
	LandedTitle *best_title = nullptr;
	LandedTitleTier best_title_tier = LandedTitleTier::Barony;

	for (LandedTitle *title : this->GetLandedTitles()) {
		if (best_title == nullptr || title->GetTier() > best_title_tier) {
			best_title = title;
			best_title_tier = title->GetTier();
		}
	}

	this->SetPrimaryTitle(best_title);
}

void Character::add_landed_title(LandedTitle *title)
{
	this->LandedTitles.push_back(title);

	//if the new title belongs to a higher tier than the current primary title (or if there is no current primary title), change the character's primary title to the new title
	if (this->GetPrimaryTitle() == nullptr || this->GetPrimaryTitle()->GetTier() < title->GetTier()) {
		this->SetPrimaryTitle(title);
	}
}

void Character::remove_landed_title(LandedTitle *title)
{
	this->LandedTitles.erase(std::remove(this->LandedTitles.begin(), this->LandedTitles.end(), title), this->LandedTitles.end());

	if (title == this->GetPrimaryTitle()) {
		this->ChoosePrimaryTitle(); //needs to choose a new primary title, as the old one has been lost
	}
}

QVariantList Character::GetTraitsQVariantList() const
{
	return util::container_to_qvariant_list(this->GetTraits());
}

/**
**	@brief	Get whether the character can build in a holding
**
**	@param	holding	The holding
*/
bool Character::can_build_in_holding(const holding *holding)
{
	return holding->get_owner() == this || this->IsAnyLiegeOf(holding->get_owner());
}

bool Character::can_build_in_holding(const QVariant &holding_variant)
{
	QObject *holding_object = qvariant_cast<QObject *>(holding_variant);
	const holding *holding = static_cast<metternich::holding *>(holding_object);
	return this->can_build_in_holding(holding);
}

}
