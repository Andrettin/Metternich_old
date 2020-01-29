#include "character/enemy.h"

#include "translator.h"

namespace metternich {

std::string enemy::get_name_plural() const
{
	return translator::get()->translate(this->get_identifier_with_aliases(), {{"plural"}});
}

}
