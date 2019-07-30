#include "history.h"

#include "map/province.h"

namespace Metternich {

void History::Load()
{
	for (Province *province : Province::GetAll()) {
		province->LoadHistory();
	}
}

}
