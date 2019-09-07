#pragma once

#include "script/modifier_base.h"

namespace metternich {

class gsml_data;
class gsml_property;

/**
**	@brief	A modifier (i.e. a collection of modifier effects)
*/
class Modifier : public ModifierBase
{
public:
	void ProcessGSMLProperty(const gsml_property &property);
	void ProcessGSMLScope(const gsml_data &scope) { Q_UNUSED(scope); }
};

}
