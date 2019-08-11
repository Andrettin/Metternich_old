#include "history.h"

#include "character/character.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/region.h"

namespace Metternich {

void History::Load()
{
	Region::ParseHistoryDatabase();
	Province::ParseHistoryDatabase();
	Character::ParseHistoryDatabase();
	LandedTitle::ParseHistoryDatabase();

	Character::ProcessHistoryDatabase(true);

	Region::ProcessHistoryDatabase(false);
	Province::ProcessHistoryDatabase(false);
	Character::ProcessHistoryDatabase(false);
	LandedTitle::ProcessHistoryDatabase(false);

	Province::InitializeAll();
	Region::InitializeAll();
	Character::InitializeAll();
	LandedTitle::InitializeAll();

	Character::CheckAll();
	LandedTitle::CheckAll();
	Province::CheckAll();
	Region::CheckAll();
}

}
