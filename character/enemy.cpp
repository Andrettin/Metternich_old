#include "character/enemy.h"

#include "util/translator.h"

namespace metternich {

std::string enemy::get_name_plural() const
{
	return translator::get()->translate(this->get_identifier_with_aliases(), {{"plural"}});
}

}
