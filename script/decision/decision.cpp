#include "script/decision/decision.h"

#include "util/translator.h"

namespace metternich {

std::string decision::get_description() const
{
	return translator::get()->translate(this->get_identifier() + "_desc");
}

}
