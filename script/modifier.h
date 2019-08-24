#pragma once

#include "script/modifier_base.h"

namespace Metternich {

class GSMLData;
class GSMLProperty;

/**
**	@brief	A modifier (i.e. a collection of modifier effects)
*/
class Modifier : public ModifierBase
{
public:
	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const GSMLData &scope) { Q_UNUSED(scope); }
};

}
