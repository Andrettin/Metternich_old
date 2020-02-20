#include "map/star_system.h"

#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/map_edge.h"
#include "map/world.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/vector_util.h"

namespace metternich {

star_system::star_system(const std::string &identifier) : data_entry(identifier), map_edge(map_edge::none)
{
}

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

void star_system::calculate_territory_polygon()
{
	//create a polygon representing the star system's territory, using the middle points between this system's primary star and the primary stars of adjacent systems to build the polygon
	this->territory_polygon.clear();

	if (this->adjacent_systems.empty()) {
		return;
	}

	const QPointF center_pos = this->get_primary_star()->get_cosmic_map_pos();

	const star_system *prev_adj_system = this->adjacent_systems.back();
	QPointF prev_adj_center_pos = prev_adj_system->get_primary_star()->get_cosmic_map_pos();
	QPointF prev_middle_pos((center_pos.x() + prev_adj_center_pos.x()) / 2, (center_pos.y() + prev_adj_center_pos.y()) / 2);
	for (const star_system *adj_system : this->adjacent_systems) {
		const QPointF adj_center_pos = adj_system->get_primary_star()->get_cosmic_map_pos();
		const QPointF middle_pos((center_pos.x() + adj_center_pos.x()) / 2, (center_pos.y() + adj_center_pos.y()) / 2);

		if (this->get_map_edge() != map_edge::none && adj_system->get_map_edge() != map_edge::none && prev_adj_system->get_map_edge() != map_edge::none) {
			QPointF prev_edge_middle_pos;
			if (is_north_map_edge(prev_adj_system->get_map_edge())) {
				prev_edge_middle_pos = QPointF(prev_middle_pos.x(), map::get()->get_cosmic_map_bounding_rect().top());
			} else if (is_south_map_edge(prev_adj_system->get_map_edge())) {
				prev_edge_middle_pos = QPointF(prev_middle_pos.x(), map::get()->get_cosmic_map_bounding_rect().bottom());
			} else if (is_west_map_edge(prev_adj_system->get_map_edge())) {
				prev_edge_middle_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().left(), prev_middle_pos.y());
			} else if (is_east_map_edge(prev_adj_system->get_map_edge())) {
				prev_edge_middle_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().right(), prev_middle_pos.y());
			}
			this->territory_polygon.append(prev_edge_middle_pos);

			if (is_diagonal_map_edge(this->get_map_edge())) {
				QPointF diagonal_edge_pos;
				if (this->get_map_edge() == map_edge::northwest) {
					diagonal_edge_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().left(), map::get()->get_cosmic_map_bounding_rect().top());
				} else if (this->get_map_edge() == map_edge::northeast) {
					diagonal_edge_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().right(), map::get()->get_cosmic_map_bounding_rect().top());
				} else if (this->get_map_edge() == map_edge::southwest) {
					diagonal_edge_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().left(), map::get()->get_cosmic_map_bounding_rect().bottom());
				} else if (this->get_map_edge() == map_edge::southeast) {
					diagonal_edge_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().right(), map::get()->get_cosmic_map_bounding_rect().bottom());
				}
				this->territory_polygon.append(diagonal_edge_pos);
			}

			QPointF edge_middle_pos;
			if (is_north_map_edge(adj_system->get_map_edge())) {
				edge_middle_pos = QPointF(middle_pos.x(), map::get()->get_cosmic_map_bounding_rect().top());
			} else if (is_south_map_edge(adj_system->get_map_edge())) {
				edge_middle_pos = QPointF(middle_pos.x(), map::get()->get_cosmic_map_bounding_rect().bottom());
			} else if (is_west_map_edge(adj_system->get_map_edge())) {
				edge_middle_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().left(), middle_pos.y());
			} else if (is_east_map_edge(adj_system->get_map_edge())) {
				edge_middle_pos = QPointF(map::get()->get_cosmic_map_bounding_rect().right(), middle_pos.y());
			}
			this->territory_polygon.append(edge_middle_pos);

		} else {
			//calculate the middle point between the previous adjacent system, the current one and this system
			const QPointF adj_middle_pos((center_pos.x() + prev_adj_center_pos.x() + adj_center_pos.x()) / 3, (center_pos.y() + prev_adj_center_pos.y() + adj_center_pos.y()) / 3);
			this->territory_polygon.append(adj_middle_pos);
		}

		this->territory_polygon.append(middle_pos);

		prev_adj_system = adj_system;
		prev_adj_center_pos = adj_center_pos;
		prev_middle_pos = middle_pos;
	}

	if (this->territory_polygon.size() >= 3) {
		this->territory_polygon.append(this->territory_polygon.front()); //close the polyon
	} else {
		this->territory_polygon.clear(); //must have at least three points to form the territory polygon
	}
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
