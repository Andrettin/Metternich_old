#include "landed_title/landed_title.h"

#include "character/character.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "game/game.h"
#include "holding/holding.h"
#include "landed_title/landed_title_tier.h"
#include "map/province.h"
#include "translator.h"

#include <stdexcept>

namespace Metternich {

/**
**	@brief	Add a new instance of the class
**
**	@param	identifier	The instance's identifier
**
**	@return	The new instance
*/
LandedTitle *LandedTitle::Add(const std::string &identifier)
{
	LandedTitle *title = DataType<LandedTitle>::Add(identifier);

	std::string identifier_prefix = identifier.substr(0, 2);

	//set the title's tier depending on the prefix of its identifier
	if (identifier_prefix == LandedTitle::BaronyPrefix) {
		title->Tier = LandedTitleTier::Barony;
	} else if (identifier_prefix == LandedTitle::CountyPrefix) {
		title->Tier = LandedTitleTier::County;
	} else if (identifier_prefix == LandedTitle::DuchyPrefix) {
		title->Tier = LandedTitleTier::Duchy;
	} else if (identifier_prefix == LandedTitle::KingdomPrefix) {
		title->Tier = LandedTitleTier::Kingdom;
	} else if (identifier_prefix == LandedTitle::EmpirePrefix) {
		title->Tier = LandedTitleTier::Empire;
	} else {
		throw std::runtime_error("Invalid identifier for new landed title: \"" + identifier + "\". Landed title identifiers must begin with one of the \"" + LandedTitle::BaronyPrefix + "\", \"" + LandedTitle::CountyPrefix + "\", \"" + LandedTitle::DuchyPrefix + "\", \"" + LandedTitle::KingdomPrefix + "\" or \"" + LandedTitle::EmpirePrefix + "\" prefixes, depending on the title's tier.");
	}

	return title;
}

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
**	@param	date		The date of the property change
*/
void LandedTitle::ProcessGSMLDatedProperty(const GSMLProperty &property, const QDateTime &date)
{
	Q_UNUSED(date);

	if (property.GetKey() == "holder") {
		if (property.GetOperator() != GSMLOperator::Assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.GetKey() + "\" property.");
		}

		if (property.GetValue() == "random") {
			//generate random holder
			Character *holder = Character::Generate(this->GetCapitalProvince()->GetCulture(), this->GetCapitalProvince()->GetReligion());
			this->SetHolder(holder);
			return;
		} else {
			const Character *holder = Character::Get(std::stoi(property.GetValue()));
			if (!holder->IsAlive()) {
				return;
			}
		}
	}

	this->ProcessGSMLProperty(property);
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void LandedTitle::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();
	const std::vector<std::string> &values = scope.GetValues();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for landed titles needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->Color.setRgb(red, green, blue);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Initialize the landed title's history
*/
void LandedTitle::InitializeHistory()
{
	if (this->HolderTitle != nullptr) {
		if (this->HolderTitle->GetHolder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->HolderTitle->GetIdentifier() + "\" holder title for \"" + this->GetIdentifier() + "\", but the former has no holder.");
		}

		this->SetHolder(this->HolderTitle->GetHolder());
		this->HolderTitle = nullptr;
	}

	if (this->LiegeTitle != nullptr) {
		if (this->LiegeTitle->GetHolder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->LiegeTitle->GetIdentifier() + "\" liege title for \"" + this->GetIdentifier() + "\", but the former has no holder.");
		}

		if (this->GetHolder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->LiegeTitle->GetIdentifier() + "\" liege title for \"" + this->GetIdentifier() + "\", but the latter has no holder.");
		}

		this->GetHolder()->SetLiege(this->LiegeTitle->GetHolder());
		this->LiegeTitle = nullptr;
	}
}

/**
**	@brief	Check whether the landed title is in a valid state
*/
void LandedTitle::Check() const
{
	if (this->GetTier() != LandedTitleTier::Barony && !this->GetColor().isValid()) {
		throw std::runtime_error("Landed title \"" + this->GetIdentifier() + "\" has no valid color.");
	}

	if (this->GetProvince() != nullptr && this->GetProvince() != this->GetCapitalProvince()) {
		throw std::runtime_error("Landed title \"" + this->GetIdentifier() + "\" has a different province and capital province.");
	}

	if (Game::Get()->IsStarting()) {
		if (this->GetCapitalProvince() == nullptr) {
			throw std::runtime_error("Landed title \"" + this->GetIdentifier() + "\" has no capital province.");
		}

		if (this->GetHolding() != nullptr && this->GetHolding()->GetProvince() != this->GetCapitalProvince()) {
			throw std::runtime_error("Landed title \"" + this->GetIdentifier() + "\" has its holding in a different province than its capital province.");
		}

		if (this->GetProvince() != nullptr) {
			if (this->GetTier() != LandedTitleTier::County) {
				throw std::runtime_error("Landed title \"" + this->GetIdentifier() + "\" has been assigned to a province, but is not a county.");
			}
		}

		if (this->GetHolding() != nullptr) {
			if (this->GetTier() != LandedTitleTier::Barony) {
				throw std::runtime_error("Landed title \"" + this->GetIdentifier() + "\" has been assigned to a holding, but is not a barony.");
			}
		}
	}
}

/**
**	@brief	Get the landed title's name
**
**	@return	The landed title's name
*/
std::string LandedTitle::GetName() const
{
	const Culture *culture = nullptr;

	if (this->GetHolder() != nullptr) {
		culture = this->GetHolder()->GetCulture();
	} else if (this->GetCapitalProvince() != nullptr) {
		culture = this->GetCapitalProvince()->GetCulture();
	}

	std::vector<std::string> suffixes;
	if (culture != nullptr) {
		suffixes.push_back(culture->GetIdentifier());
		suffixes.push_back(culture->GetCultureGroup()->GetIdentifier());
	}

	return Translator::Get()->Translate(this->GetIdentifier(), suffixes);
}

void LandedTitle::SetHolder(Character *character)
{
	if (character == this->GetHolder()) {
		return;
	}

	if (this->GetHolder() != nullptr) {
		this->GetHolder()->RemoveLandedTitle(this);
	}

	this->Holder = character;
	character->AddLandedTitle(this);
	this->HolderTitle = nullptr; //set the holder title to null, so that the new holder isn't overwritten by a previous holder title

	//if this is a non-titular county, then the character holding it must also possess the county's capital holding
	if (this->GetProvince() != nullptr) {
		this->GetProvince()->GetCapitalHolding()->GetBarony()->SetHolder(character);
	}

	//if this title is associated with a holding (i.e. it is a non-titular barony), then its holder must also be the owner of the holding
	if (this->GetHolding() != nullptr) {
		this->GetHolding()->SetOwner(character);
	}

	emit HolderChanged();
}

void LandedTitle::SetHolding(Metternich::Holding *holding)
{
	this->Holding = holding;

	if (holding != nullptr) {
		this->CapitalProvince = holding->GetProvince();
	} else {
		this->CapitalProvince = nullptr;
	}
}

LandedTitle *LandedTitle::GetRealm() const
{
	Character *holder = this->GetHolder();

	if (holder != nullptr) {
		Character *top_liege = holder->GetTopLiege();
		return top_liege->GetPrimaryTitle();
	}

	return nullptr;
}

void LandedTitle::SetHolderTitle(LandedTitle *title)
{
	this->HolderTitle = title;
	this->Holder = nullptr; //set the holder title to null, so that the new holder title isn't overwritten by a previous holder

}

void LandedTitle::SetDeJureLiegeTitle(LandedTitle *title)
{
	if (title == this->GetDeJureLiegeTitle()) {
		return;
	}

	if (this->GetDeJureLiegeTitle() != nullptr) {
		this->GetDeJureLiegeTitle()->RemoveDeJureVassalTitle(this);
	}

	if (static_cast<int>(title->GetTier()) - static_cast<int>(this->GetTier()) != 1) {
		throw std::runtime_error("Tried to set title \"" + title->GetIdentifier() + "\" as the de jure liege of \"" + this->GetIdentifier() + "\", but the former is not one title tier above the latter.");
	}

	this->DeJureLiegeTitle = title;
	title->AddDeJureVassalTitle(this);

	emit DeJureLiegeTitleChanged();
}

}
