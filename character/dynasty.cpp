#include "character/dynasty.h"

#include "culture/culture.h"

namespace metternich {

void dynasty::set_culture(metternich::culture *culture)
{
	this->culture = culture;
	culture->add_dynasty(this);
}

}
