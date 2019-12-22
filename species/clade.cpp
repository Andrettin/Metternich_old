#include "species/clade.h"

#include "game/game.h"

namespace metternich {

bool clade::is_ai() const
{
	return game::get()->get_player_clade() != this;
}

}
