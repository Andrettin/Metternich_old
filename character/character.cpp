#include "character/character.h"

#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "database/gsml_property.h"
#include "game/game.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "landed_title/landed_title_tier.h"
#include "random.h"
#include "translator.h"
#include "util.h"

#include <QVariant>

namespace metternich {

/**
**	@brief	Generate a character
**
**	@param	culture		The culture for the generated character
**
**	@param	religion	The religion for the generated character
*/
Character *Character::Generate(metternich::Culture *culture, metternich::Religion *religion)
{
	const int identifier = Character::GenerateNumericIdentifier();
	Character *character = Character::Add(identifier);
	character->Culture = culture;
	character->Religion = religion;
	character->name = culture->GenerateMaleName();
	//generate the character's birth date to be between 60 and 20 years before the current date
	const QDateTime &current_date = Game::Get()->GetCurrentDate();
	character->BirthDate = current_date.addDays(Random::GenerateInRange(-60 * 365, -20 * 365));
	return character;
}

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
**	@param	date		The date of the property change
*/
void Character::ProcessGSMLDatedProperty(const gsml_property &property, const QDateTime &date)
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
		this->ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Initialize the character's history
*/
void Character::initialize_history()
{
	if (this->name.empty() && this->GetCulture() != nullptr) {
		if (this->IsFemale()) {
			this->name = this->GetCulture()->GenerateFemaleName();
		} else {
			this->name = this->GetCulture()->GenerateMaleName();
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

void Character::AddLandedTitle(LandedTitle *title)
{
	this->LandedTitles.push_back(title);

	//if the new title belongs to a higher tier than the current primary title (or if there is no current primary title), change the character's primary title to the new title
	if (this->GetPrimaryTitle() == nullptr || this->GetPrimaryTitle()->GetTier() < title->GetTier()) {
		this->SetPrimaryTitle(title);
	}
}

void Character::RemoveLandedTitle(LandedTitle *title)
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
