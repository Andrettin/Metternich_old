#include "history.h"

#include "character.h"
#include "landed_title.h"
#include "map/province.h"

namespace Metternich {

void History::Load()
{
	Province::LoadHistoryDatabase();
	Character::LoadHistoryDatabase();
	LandedTitle::LoadHistoryDatabase();

	Character::CheckAll();
	LandedTitle::CheckAll();
	Province::CheckAll();
}

}
