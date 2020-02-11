#include "map/star_system.h"

#include "map/world.h"
#include "util/container_util.h"
#include "util/point_util.h"
#include "util/vector_util.h"

namespace metternich {

void star_system::initialize()
{
	double latitude = 0;
	double longitude = 0;
	long int astrodistance = 0;
	int star_count = 0;

	for (world *world : this->worlds) {
		if (world->is_star() && world->get_astrocoordinate().isValid()) {
			latitude += world->get_astrocoordinate().latitude();
			longitude += world->get_astrocoordinate().longitude();
			astrodistance += world->get_astrodistance();
			star_count++;
		}
	}

	if (star_count > 0) {
		this->astrocoordinate = QGeoCoordinate(latitude / star_count, longitude / star_count);
		this->astrodistance = static_cast<int>(astrodistance / star_count);
	}

	std::sort(this->worlds.begin(), this->worlds.end(), [](const world *a, const world *b) {
		return a->get_distance_from_orbit_center() < b->get_distance_from_orbit_center();
	});

	//update the world distances from the system center so that orbits are within a minimum and maximum distance of each other
	const world *previous_world = nullptr;
	for (world *world : this->worlds) {
		world->calculate_cosmic_pixel_size();

		if (previous_world != nullptr) {
			const int previous_distance = previous_world->get_distance_from_orbit_center();
			const int base_distance = previous_distance + (previous_world->get_cosmic_pixel_size() / 2) + (world->get_cosmic_pixel_size() / 2);
			const int min_distance = base_distance + star_system::min_orbit_distance;
			const int max_distance = base_distance + star_system::max_orbit_distance;
			int distance = world->get_distance_from_orbit_center();
			distance = std::max(distance, min_distance);
			distance = std::min(distance, max_distance);
			world->set_distance_from_orbit_center(distance);
		}

		previous_world = world;
	}

	data_entry_base::initialize();
}

QVariantList star_system::get_worlds_qvariant_list() const
{
	return container::to_qvariant_list(this->get_worlds());
}

void star_system::remove_world(world *world)
{
	vector::remove(this->worlds, world);
}

}
