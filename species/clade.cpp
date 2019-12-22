#include "species/clade.h"

#include "game/game.h"
#include "map/province.h"
#include "util/container_util.h"

namespace metternich {

QVariantList clade::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

bool clade::is_ai() const
{
	return game::get()->get_player_clade() != this;
}

}
