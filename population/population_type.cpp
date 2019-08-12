#include "population/population_type.h"

#include "holding/holding_type.h"
#include "util.h"

namespace Metternich {

QVariantList PopulationType::GetHoldingTypesQVariantList() const
{
	return ContainerToQVariantList(this->GetHoldingTypes());
}

}
