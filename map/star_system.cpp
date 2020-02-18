#include "map/star_system.h"

#include "map/world.h"
#include "util/container_util.h"
#include "util/vector_util.h"

namespace metternich {

void star_system::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "adjacent_systems") {
		this->adjacent_systems.clear();
		for (const std::string &value : values) {
			this->adjacent_systems.push_back(star_system::get(value));
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void star_system::initialize()
{
	if (this->get_primary_star() == nullptr) {
		this->calculate_primary_star();
	}

	//create a polygon representing the star system's territory, using the middle points between this system's primary star and the primary stars of adjacent systems to build the polygon
	this->territory_polygon.clear();

	const QPointF center_pos = this->get_primary_star()->get_cosmic_map_pos();
	for (star_system *adjacent_system : this->adjacent_systems) {
		if (adjacent_system->get_primary_star() == nullptr) {
			adjacent_system->calculate_primary_star();
		}

		const QPointF adjacent_center_pos = adjacent_system->get_primary_star()->get_cosmic_map_pos();
		const QPointF middle_pos((center_pos.x() + adjacent_center_pos.x()) / 2, (center_pos.y() + adjacent_center_pos.y()) / 2);
		this->territory_polygon.append(middle_pos);
	}

	if (this->territory_polygon.size() >= 3) {
		this->territory_polygon.append(this->territory_polygon.front()); //close the polyon
	} else {
		this->territory_polygon.clear(); //must have at least three points to form the territory polygon
	}
}

void star_system::calculate_primary_star()
{
	this->primary_star = nullptr;

	for (world *world : this->get_worlds()) {
		if (world->get_orbit_center() == nullptr) {
			if (this->get_primary_star() != nullptr) {
				throw std::runtime_error("Star system \"" + this->get_identifier() + "\" has two celestial bodies orbiting no other one, making it impossible to determine its primary star.");
			}

			this->primary_star = world;
		}
	}

	if (this->get_primary_star() == nullptr) {
		throw std::runtime_error("Star system \"" + this->get_identifier() + "\" has no celestial bodies orbiting no other one, and thus no candidate for primary star.");
	}
}

QVariantList star_system::get_territory_polygon_qvariant_list() const
{
	const QRectF bounding_rect = this->get_territory_bounding_rect();
	const QPolygonF polygon = this->territory_polygon.translated(QPointF(bounding_rect.x() * -1, bounding_rect.y() * -1));

	QVariantList polygon_qvariant_list;
	for (const QPointF &pos : polygon) {
		polygon_qvariant_list.append(QVariant::fromValue(QPointF(pos.x(), bounding_rect.height() - pos.y())));
	}

	return polygon_qvariant_list;
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
