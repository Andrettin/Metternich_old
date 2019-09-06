#include "character/dynasty.h"

#include "culture/culture.h"

namespace metternich {

void Dynasty::SetCulture(metternich::Culture *culture)
{
	this->Culture = culture;
	culture->AddDynasty(this);
}

}
