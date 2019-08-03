#include "history.h"

#include "character.h"
#include "landed_title.h"
#include "map/province.h"

namespace Metternich {

void History::Load()
{
	Province::ParseHistoryDatabase();
	Character::ParseHistoryDatabase();
	LandedTitle::ParseHistoryDatabase();

	Character::ProcessHistoryDatabase(true);

	Province::ProcessHistoryDatabase(false);
	Character::ProcessHistoryDatabase(false);
	LandedTitle::ProcessHistoryDatabase(false);

	Character::InitializeAll();

	Character::CheckAll();
	LandedTitle::CheckAll();
	Province::CheckAll();
}

}
