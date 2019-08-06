#include "character/dynasty.h"

#include "culture/culture.h"

namespace Metternich {

void Dynasty::SetCulture(Metternich::Culture *culture)
{
	this->Culture = culture;
	culture->AddDynasty(this);
}

}
