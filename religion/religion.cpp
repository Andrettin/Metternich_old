#include "religion/religion.h"

#include "religion/religion_group.h"

namespace metternich {

void religion::check() const
{
	if (this->get_religion_group() == nullptr) {
		throw std::runtime_error("Religion \"" + this->get_identifier() + "\" has no religion group.");
	}

	if (!this->get_color().isValid()) {
		throw std::runtime_error("Religion \"" + this->get_identifier() + "\" has no valid color.");
	}
}

}
