#pragma once

#include "script/modifier_base.h"

namespace metternich {

class gsml_data;
class gsml_property;

/**
**	@brief	A modifier (i.e. a collection of modifier effects)
*/
class modifier : public modifier_base
{
public:
	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope) { Q_UNUSED(scope); }
};

}
