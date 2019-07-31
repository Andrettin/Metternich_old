#include "history.h"

#include "character.h"
#include "map/province.h"

namespace Metternich {

void History::Load()
{
	Province::LoadHistoryDatabase();
	Character::LoadHistoryDatabase();
}

}
