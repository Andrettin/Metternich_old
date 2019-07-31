#include "landed_title.h"

#include "character.h"
#include "culture.h"
#include "landed_title_tier.h"
#include "map/province.h"
#include "translator.h"

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
*/
void LandedTitle::ProcessGSMLHistoryProperty(const GSMLProperty &property, const QDateTime &date)
{
	Q_UNUSED(date);

	if (property.GetKey() == "holder") {
		if (property.GetOperator() != GSMLOperator::Assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.GetKey() + "\" property.");
		}

		const Character *holder = Character::Get(std::stoi(property.GetValue()));
		if (!holder->IsAlive()) {
			return;
		}
	}

	this->ProcessGSMLProperty(property);
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
	} else if (this->GetProvince() != nullptr) {
		culture = this->GetProvince()->GetCulture();
	}

	std::vector<std::string> suffixes;
	if (culture != nullptr) {
		suffixes.push_back(culture->GetIdentifier());
	}

	return Translator::GetInstance()->Translate(this->GetIdentifier(), suffixes);
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

	emit HolderChanged();
}

}
