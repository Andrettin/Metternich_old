#include "chance_factor.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
template <typename T>
void ChanceFactor<T>::ProcessGSMLProperty(const GSMLProperty &property)
{
	const std::string &key = property.GetKey();
	const GSMLOperator gsml_operator = property.GetOperator();
	const std::string &value = property.GetValue();

	if (key == "factor") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->Factor = CentesimalNumberStringToInt(value);
		} else {
			throw std::runtime_error("Invalid operator for property (\"" + property.GetKey() + "\").");
		}
	} else {
		throw std::runtime_error("Invalid chance factor property: \"" + property.GetKey() + "\".");
	}
}

//explicit instantiations of the template class
class Holding;

template class ChanceFactor<Holding>;

}
