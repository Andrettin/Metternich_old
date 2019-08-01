#include "dynasty.h"

#include "culture.h"

namespace Metternich {

void Dynasty::SetCulture(Metternich::Culture *culture)
{
	this->Culture = culture;
	culture->AddDynasty(this);
}

}
