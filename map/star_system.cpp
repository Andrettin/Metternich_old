#include "map/star_system.h"

#include "landed_title/landed_title.h"
#include "map/world.h"
#include "translator.h"
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

	for (star_system *adj_system : this->adjacent_systems) {
		if (adj_system->get_primary_star() == nullptr) {
			adj_system->calculate_primary_star();
		}
	}

	if (!this->adjacent_systems.empty()) {
		const star_system *prev_adj_system = this->adjacent_systems.back();
		QPointF prev_adj_center_pos = prev_adj_system->get_primary_star()->get_cosmic_map_pos();
		for (const star_system *adj_system : this->adjacent_systems) {
			const QPointF adj_center_pos = adj_system->get_primary_star()->get_cosmic_map_pos();
			const QPointF middle_pos((center_pos.x() + adj_center_pos.x()) / 2, (center_pos.y() + adj_center_pos.y()) / 2);

			if (prev_adj_system != nullptr && prev_adj_system != adj_system) {
				//calculate the middle point between the previous adjacent system, the current one and this system
				const QPointF adj_middle_pos((center_pos.x() + prev_adj_center_pos.x() + adj_center_pos.x()) / 3, (center_pos.y() + prev_adj_center_pos.y() + adj_center_pos.y()) / 3);
				this->territory_polygon.append(adj_middle_pos);
			}

			this->territory_polygon.append(middle_pos);
			prev_adj_system = adj_system;
			prev_adj_center_pos = adj_center_pos;
		}
	}

	if (this->territory_polygon.size() >= 3) {
		this->territory_polygon.append(this->territory_polygon.front()); //close the polyon
	} else {
		this->territory_polygon.clear(); //must have at least three points to form the territory polygon
	}
}

std::string star_system::get_name() const
{
	if (this->get_duchy() != nullptr) {
		return translator::get()->translate(this->get_duchy()->get_identifier_with_aliases());
	}

	return translator::get()->translate(this->get_identifier_with_aliases()); //star system without a cosmic duchy
}

void star_system::set_duchy(landed_title *duchy)
{
	if (duchy == this->get_duchy()) {
		return;
	}

	this->duchy = duchy;
	duchy->set_star_system(this);
	emit duchy_changed();
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

const QColor &star_system::get_color() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_color();
	}

	return star_system::empty_system_color;
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
