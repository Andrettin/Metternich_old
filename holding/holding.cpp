#include "holding.h"

#include "culture.h"
#include "landed_title.h"
#include "map/province.h"
#include "religion.h"
#include "translator.h"

namespace Metternich {

Holding::Holding(LandedTitle *barony, HoldingType *type, Metternich::Province *province) : Barony(barony), Type(type), Province(province)
{
	barony->SetHolding(this);
}

Holding::~Holding()
{
	this->Barony->SetHolding(nullptr);
}

/**
**	@brief	Get the holding's name
**
**	@return	The holding's name
*/
std::string Holding::GetName() const
{
	return Translator::GetInstance()->Translate(this->GetBarony()->GetIdentifier(), {this->GetProvince()->GetCulture()->GetIdentifier(), this->GetProvince()->GetReligion()->GetIdentifier()});
}

}
