#include "character.h"

#include "database/gsml_property.h"
#include "util.h"

namespace Metternich {

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

}
