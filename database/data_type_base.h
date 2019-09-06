#pragma once

#include "database/gsml_data.h"

#include <vector>

namespace Metternich {

/**
**	@brief	The base class for the data type class per se, and for simpler data type classes which don't use identifiers
*/
template <typename T>
class DataTypeBase
{
protected:
	static inline std::vector<gsml_data> gsml_history_data_to_process;
};

}
