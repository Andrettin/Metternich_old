#include "character.h"

#include "database/gsml_property.h"
#include "landed_title.h"
#include "landed_title_tier.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
*/
void Character::ProcessGSMLHistoryProperty(const GSMLProperty &property, const QDateTime &date)
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

}
