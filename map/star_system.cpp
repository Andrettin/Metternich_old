#include "map/star_system.h"

#include "map/world.h"
#include "util/container_util.h"
#include "util/vector_util.h"

namespace metternich {

QVariantList star_system::get_worlds_qvariant_list() const
{
	return container::to_qvariant_list(this->get_worlds());
}

void star_system::remove_world(world *world)
{
	vector::remove(this->worlds, world);
}

}
