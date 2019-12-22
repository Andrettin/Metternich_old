#include "religion/religion_group.h"

namespace metternich {

void religion_group::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Religion group \"" + this->get_identifier() + "\" has no valid color.");
	}
}

}
