#include "history.h"

#include "character/character.h"
#include "landed_title/landed_title.h"
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
	LandedTitle::InitializeAll();

	Character::CheckAll();
	LandedTitle::CheckAll();
	Province::CheckAll();
}

}
