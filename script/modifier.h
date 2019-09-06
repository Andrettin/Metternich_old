#pragma once

#include "script/modifier_base.h"

namespace Metternich {

class gsml_data;
class GSMLProperty;

/**
**	@brief	A modifier (i.e. a collection of modifier effects)
*/
class Modifier : public ModifierBase
{
public:
	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const gsml_data &scope) { Q_UNUSED(scope); }
};

}
