#include "character/character.h"

#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "database/gsml_property.h"
#include "game/game.h"
#include "landed_title/landed_title.h"
#include "landed_title/landed_title_tier.h"
#include "random.h"
#include "util.h"

#include <QVariant>

namespace Metternich {

/**
**	@brief	Generate a character
**
**	@param	culture		The culture for the generated character
**
**	@param	religion	The religion for the generated character
*/
Character *Character::Generate(Metternich::Culture *culture, Metternich::Religion *religion)
{
	const int identifier = Character::GenerateNumericIdentifier();
	Character *character = Character::Add(identifier);
	character->Culture = culture;
	character->Religion = religion;
	character->Name = culture->GenerateMaleName();
	//generate the character's birth date to be between 60 and 20 years before the current date
	const QDateTime &current_date = Game::GetInstance()->GetCurrentDate();
	character->BirthDate = current_date.addDays(Random::GenerateInRange(-60 * 365, -20 * 365));
	return character;
}

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
*/
void Character::ProcessGSMLDatedProperty(const GSMLProperty &property, const QDateTime &date)
{
	if (property.GetKey() == "birth") {
		if (property.GetOperator() != GSMLOperator::Assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.GetKey() + "\" property.");
		}

		if (StringToBool(property.GetValue())) {
			this->BirthDate = date;
			this->Alive = true;
		}
	} else if (property.GetKey() == "death") {
		if (property.GetOperator() != GSMLOperator::Assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.GetKey() + "\" property.");
		}

		if (StringToBool(property.GetValue())) {
			this->DeathDate = date;
			this->Alive = false;
		}
	} else {
		this->ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Initialize the character
*/
void Character::Initialize()
{
	if (this->Name.empty() && this->GetCulture() != nullptr) {
		if (this->IsFemale()) {
			this->Name = this->GetCulture()->GenerateFemaleName();
		} else {
			this->Name = this->GetCulture()->GenerateMaleName();
		}
	}
}

std::string Character::GetFullName() const
{
	std::string full_name = this->Name;
	if (this->GetDynasty() != nullptr) {
		full_name += " " + this->GetDynasty()->GetName();
	}
	return full_name;
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
	return VectorToQVariantList(this->GetTraits());
}

}
