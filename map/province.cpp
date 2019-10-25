#include "map/province.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/region.h"
#include "map/terrain_type.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "religion.h"
#include "script/modifier.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/location_util.h"
#include "util/point_util.h"

#include <QApplication>
#include <QGeoCircle>
#include <QPainter>

namespace metternich {

/**
**	@brief	Get the string identifiers of the classes on which this one depends for loading its database
**
**	@return	The class identifier string list
*/
std::set<std::string> province::get_database_dependencies()
{
	return {
		//so that baronies will be ensured to exist when provinces (and thus holding slots) are processed
		landed_title::class_identifier,
		//so that the effects of the set_terrain() function can occur properly
		terrain_type::class_identifier
	};
}

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**	@param	rgb	The instance's RGB
**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error)
**	@return	The instance if found, or null otherwise
*/
province *province::get_by_rgb(const QRgb &rgb, const bool should_find)
{
	if (rgb == province::empty_rgb) {
		return nullptr;
	}

	typename std::map<QRgb, province *>::const_iterator find_iterator = province::instances_by_rgb.find(rgb);

	if (find_iterator != province::instances_by_rgb.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		QColor color(rgb);
		throw std::runtime_error("No province found for RGB value: " + std::to_string(color.red()) + "/" + std::to_string(color.green()) + "/" + std::to_string(color.blue()) + ".");
	}

	return nullptr;
}

/**
**	@brief	Add a new instance of the class
**
**	@param	identifier	The instance's identifier
**
**	@return	The new instance
*/
province *province::add(const std::string &identifier)
{
	if (identifier.substr(0, 2) != province::prefix) {
		throw std::runtime_error("Invalid identifier for new province: \"" + identifier + "\". Province identifiers must begin with \"" + province::prefix + "\".");
	}

	return data_type<province>::add(identifier);
}

/**
**	@brief	Constructor
*/
province::province(const std::string &identifier) : data_entry(identifier)
{
	connect(this, &province::culture_changed, this, &identifiable_data_entry_base::name_changed);
	connect(this, &province::religion_changed, this, &identifiable_data_entry_base::name_changed);
	connect(Game::get(), &Game::RunningChanged, this, &province::update_image);
	connect(this, &province::selected_changed, this, &province::update_image);
}

/**
**	@brief	Destructor
*/
province::~province()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void province::process_gsml_property(const gsml_property &property)
{
	if (property.get_key().substr(0, 2) == holding_slot::prefix) {
		//a property related to one of the province's holdings
		holding_slot *holding_slot = holding_slot::get(property.get_key());
		holding *holding = this->get_holding(holding_slot);
		if (property.get_operator() == gsml_operator::assignment) {
			//the assignment operator sets the holding's type (creating the holding if it doesn't exist)
			holding_type *holding_type = holding_type::get(property.get_value());
			if (holding != nullptr) {
				holding->set_type(holding_type);
			} else {
				this->create_holding(holding_slot, holding_type);
			}
		} else if (property.get_operator() == gsml_operator::addition || property.get_operator() == gsml_operator::subtraction) {
			//the addition/subtraction operators add/remove buildings to/from the holding
			Building *building = Building::get(property.get_value());
			if (property.get_operator() == gsml_operator::addition) {
				holding->add_building(building);
			} else if (property.get_operator() == gsml_operator::subtraction) {
				holding->remove_building(building);
			}
		}
	} else {
		data_entry_base::process_gsml_property(property);
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void province::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for provinces needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->color.setRgb(red, green, blue);

		if (province::instances_by_rgb.find(this->color.rgb()) != province::instances_by_rgb.end()) {
			throw std::runtime_error("The color set for province \"" + this->get_identifier() + "\" is already used by province \"" + province::instances_by_rgb.find(this->color.rgb())->second->get_identifier() + "\"");
		}

		province::instances_by_rgb[this->color.rgb()] = this;
	} else if (tag == "geopolygons") {
		for (const gsml_data &polygon_data : scope.get_children()) {
			this->geopolygons.push_back(polygon_data.to_geopolygon());
		}
	} else if (tag == "geopaths") {
		for (const gsml_data &path_data : scope.get_children()) {
			QGeoPath geopath = path_data.to_geopath();
			if (this->get_terrain() != nullptr) {
				geopath.setWidth(this->get_terrain()->get_path_width());
			}
			this->geopaths.push_back(geopath);
		}
	} else if (tag == "border_provinces") {
		for (const std::string &border_province_identifier : scope.get_values()) {
			province *border_province = province::get(border_province_identifier);
			this->border_provinces.insert(border_province);
		}
	} else if (tag.substr(0, 2) == holding_slot::prefix) {
		holding_slot *holding_slot = holding_slot::add(tag, this);
		this->holding_slots.push_back(holding_slot);
		database::process_gsml_data(holding_slot, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Process a GSML history scope
**
**	@param	scope	The scope
**	@param	date	The date of the scope change
*/
void province::process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date)
{
	const std::string &tag = scope.get_tag();

	if (tag.substr(0, 2) == holding_slot::prefix) {
		//a change to the data of one of the province's holdings

		holding_slot *holding_slot = holding_slot::get(tag);
		holding *holding = this->get_holding(holding_slot);
		if (holding != nullptr) {
			for (const gsml_property &property : scope.get_properties()) {
				holding->process_gsml_dated_property(property, date);
			}
		} else {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no constructed holding for barony \"" + tag + "\", while having history to change the holding's data.");
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Initialize the province's history
*/
void province::initialize_history()
{
	this->population_units.clear();

	for (holding *holding : this->get_holdings()) {
		holding->initialize_history();
	}

	this->calculate_population();
	this->calculate_population_groups();

	if (this->has_river()) {
		this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province has a river
	}
	if (this->is_coastal()) {
		this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province is coastal
	}
}

/**
**	@brief	Check whether the province is in a valid state
*/
void province::check() const
{
	if (Game::get()->IsStarting()) {
		if (this->get_terrain() == nullptr) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no terrain.");
		}

		if (!this->get_color().isValid()) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no color.");
		}

		/*
		if (this->border_provinces.empty()) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no border provinces.");
		}
		*/

		if (this->get_county() != nullptr) {
			if (this->get_culture() == nullptr) {
				throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no culture.");
			}

			if (this->get_religion() == nullptr) {
				throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no religion.");
			}

			if (this->get_capital_holding() != nullptr && this->get_capital_holding()->get_province() != this) {
				throw std::runtime_error("Province \"" + this->get_identifier() + "\"'s capital holding (\"" + this->get_capital_holding()->get_barony()->get_identifier() + "\") belongs to another province (\"" + this->get_capital_holding()->get_province()->get_identifier() + "\").");
			}
		}
	}
}

/**
**	@brief	Get cache data for the province
*/
gsml_data province::get_cache_data() const
{
	gsml_data cache_data(this->get_identifier());
	if (this->get_terrain() != nullptr) {
		cache_data.add_property("terrain", this->get_terrain()->get_identifier());
	}

	gsml_data border_provinces("border_provinces");

	for (const province *province : this->border_provinces) {
		border_provinces.add_value(province->get_identifier());
	}

	cache_data.add_child(std::move(border_provinces));

	return cache_data;
}

/**
**	@brief	Do the province's daily actions
*/
void province::do_day()
{
	for (holding *holding : this->get_holdings()) {
		holding->do_day();
	}
}

/**
**	@brief	Do the province's monthly actions
*/
void province::do_month()
{
	for (holding *holding : this->get_holdings()) {
		holding->do_month();
	}

	this->calculate_population_groups();
}

/**
**	@brief	Get the province's name
**
**	@return	The province's name
*/
std::string province::get_name() const
{
	if (this->get_county() != nullptr) {
		return Translator::get()->Translate(this->get_county()->get_identifier(), {this->get_culture()->get_identifier(), this->get_culture()->get_culture_group()->get_identifier(), this->get_religion()->get_identifier()});
	}

	return Translator::get()->Translate(this->get_identifier()); //province without a county; sea zone, river, lake or wasteland
}

/**
**	@brief	Set the province's county
**
**	@param	county	The new county for the province
*/
void province::set_county(landed_title *county)
{
	if (county == this->get_county()) {
		return;
	}

	this->county = county;
	county->set_province(this);
	emit county_changed();
}

/**
**	@brief	Get the province's (de jure) duchy
**
**	@return	The province's (de jure) duchy
*/
landed_title *province::get_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_liege_title();
	}

	return nullptr;
}

/**
**	@brief	Get the province's (de jure) kingdom
**
**	@return	The province's (de jure) kingdom
*/
landed_title *province::get_kingdom() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_de_jure_liege_title();
	}

	return nullptr;
}

/**
**	@brief	Get the province's (de jure) empire
**
**	@return	The province's (de jure) empire
*/
landed_title *province::get_empire() const
{
	if (this->get_kingdom() != nullptr) {
		return this->get_kingdom()->get_de_jure_liege_title();
	}

	return nullptr;
}

/**
**	@brief	Create the province's image
**
**	@param	pixel_indexes	The indexes of the province's pixels
*/
void province::create_image(const std::vector<int> &pixel_indexes)
{
	QPoint start_pos(-1, -1);
	QPoint end_pos(-1, -1);

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = map::get()->get_pixel_pos(index);
		if (start_pos.x() == -1 || pixel_pos.x() < start_pos.x()) {
			start_pos.setX(pixel_pos.x());
		}
		if (start_pos.y() == -1 || pixel_pos.y() < start_pos.y()) {
			start_pos.setY(pixel_pos.y());
		}
		if (end_pos.x() == -1 || pixel_pos.x() > end_pos.x()) {
			end_pos.setX(pixel_pos.x());
		}
		if (end_pos.y() == -1 || pixel_pos.y() > end_pos.y()) {
			end_pos.setY(pixel_pos.y());
		}
	}

	this->rect = QRect(start_pos, end_pos);

	this->image = QImage(this->rect.size(), QImage::Format_Indexed8);

	//index 0 = transparency, index 1 = the main color for the province, index 2 = province border
	this->image.setColorTable({qRgba(0, 0, 0, 0), this->get_color().rgb(), QColor(Qt::darkGray).rgb()});
	this->image.fill(0);

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = map::get()->get_pixel_pos(index) - this->rect.topLeft();
		this->image.setPixel(pixel_pos, 1);
	}
}

/**
**	@brief	Set the border pixels for the province's image
**
**	@param	pixel_indexes	The indexes of the province's border pixels
*/
void province::set_border_pixels(const std::vector<int> &pixel_indexes)
{
	for (const int index : pixel_indexes) {
		QPoint pixel_pos = map::get()->get_pixel_pos(index) - this->rect.topLeft();
		this->image.setPixel(pixel_pos, 2);
	}
}

/**
**	@brief	Update the province's image
*/
void province::update_image()
{
	QColor province_color;
	if (this->get_county() != nullptr) {
		const landed_title *realm = this->get_county()->get_realm();
		if (realm != nullptr) {
			province_color = realm->get_color();
		} else {
			province_color = this->get_county()->get_color();
		}
	} else if (this->is_water()) {
		province_color = QColor("#0080ff");
	} else {
		province_color = QColor(Qt::darkGray); //wasteland
	}

	QColor border_color;
	if (this->is_selected()) {
		//if the province is selected, highlight its border pixels
		border_color = QColor(Qt::yellow);
	} else {
		border_color = QColor(Qt::black);
	}

	this->image.setColor(1, province_color.rgb());
	this->image.setColor(2, border_color.rgb());

	emit image_changed();
}

/**
**	@brief	Write the province's geodata to a province image
**
**	@param	image			The image to which the province's geodata will be written to
**	@param	terrain_image	The terrain image to be updated from the province's geodata, if the province has preset terrain
*/
void province::write_geodata_to_image(QImage &image, QImage &terrain_image)
{
	for (const QGeoPolygon &geopolygon : this->geopolygons) {
		this->write_geoshape_to_image(image, geopolygon, terrain_image);
	}

	for (const QGeoPath &geopath : this->geopaths) {
		this->write_geoshape_to_image(image, geopath, terrain_image);
	}
}

/**
**	@brief	Write the province's geopath endpoints to a province image
**
**	@param	image			The image to which the province's geodata will be written to
**	@param	terrain_image	The terrain image to be updated from the province's geodata, if the province has preset terrain
*/
void province::write_geopath_endpoints_to_image(QImage &image, QImage &terrain_image)
{
	const int circle_radius = this->get_terrain()->get_path_width() / 2;

	for (const QGeoPath &geopath : this->geopaths) {
		QGeoCircle front_geocircle(geopath.path().front(), circle_radius);
		this->write_geoshape_to_image(image, front_geocircle, terrain_image);

		QGeoCircle back_geocircle(geopath.path().back(), circle_radius);
		this->write_geoshape_to_image(image, back_geocircle, terrain_image);
	}
}

/**
**	@brief	Write a geoshape belonging to the province to an image
**
**	@param	image			The image to which the geoshape is to be written
**	@param	geoshape		The geoshape
**	@param	terrain_image	The terrain image to which the geoshape is to be written, if the province has preset terrain
*/
void province::write_geoshape_to_image(QImage &image, const QGeoShape &geoshape, QImage &terrain_image)
{
	const QString province_loading_message = EngineInterface::get()->get_loading_message();

	QRgb rgb = this->get_color().rgb();
	QRgb *rgb_data = reinterpret_cast<QRgb *>(image.bits());

	QRgb terrain_rgb;
	if (this->get_terrain() != nullptr) {
		terrain_rgb = this->get_terrain()->get_color().rgb();
	} else if (terrain_type::get_default_terrain() != nullptr) {
		terrain_rgb = terrain_type::get_default_terrain()->get_color().rgb();
	}

	QRgb *terrain_rgb_data = reinterpret_cast<QRgb *>(terrain_image.bits());

	const double lon_per_pixel = 360.0 / static_cast<double>(image.size().width());
	const double lat_per_pixel = 180.0 / static_cast<double>(image.size().height());

	QGeoRectangle georectangle = geoshape.boundingGeoRectangle();
	QGeoCoordinate bottom_left = georectangle.bottomLeft();
	QGeoCoordinate top_right = georectangle.topRight();

	if (geoshape.type() == QGeoShape::ShapeType::PathType) {
		//increase the bounding rectangle of paths slightly, as otherwise a part of the path's width is cut off
		bottom_left.setLatitude(bottom_left.latitude() - 0.1);
		bottom_left.setLongitude(bottom_left.longitude() - 0.1);
		top_right.setLatitude(top_right.latitude() + 0.1);
		top_right.setLongitude(top_right.longitude() + 0.1);
	}

	double lon = bottom_left.longitude();
	lon = std::round(lon / lon_per_pixel) * lon_per_pixel;
	const int start_x = util::longitude_to_x(lon, lon_per_pixel);

	double start_lat = bottom_left.latitude();
	start_lat = std::round(start_lat / lat_per_pixel) * lat_per_pixel;

	const int pixel_width = static_cast<int>(std::round((std::abs(top_right.longitude() - bottom_left.longitude())) / lon_per_pixel));
	const bool show_progress = pixel_width >= 512;

	for (; lon <= top_right.longitude(); lon += lon_per_pixel) {
		const int x = util::longitude_to_x(lon, lon_per_pixel);

		for (double lat = start_lat; lat <= top_right.latitude(); lat += lat_per_pixel) {
			QGeoCoordinate coordinate(lat, lon);

			const int y = util::latitude_to_y(lat, lat_per_pixel);
			const int pixel_index = util::point_to_index(x, y, image.size());

			//only write the province to the pixel if it is empty, or if this is a river province and the province to overwrite is not an ocean province
			if (rgb_data[pixel_index] != province::empty_rgb && (!this->is_river() || province::get_by_rgb(rgb_data[pixel_index])->is_ocean())) {
				continue;
			}

			if (!geoshape.contains(coordinate)) {
				continue;
			}

			rgb_data[pixel_index] = rgb;

			if (terrain_rgb_data[pixel_index] == terrain_type::empty_rgb || this->is_river()) {
				terrain_rgb_data[pixel_index] = terrain_rgb;
			}
		}

		if (show_progress) {
			const int progress_percent = (x - start_x) * 100 / pixel_width;
			EngineInterface::get()->set_loading_message(province_loading_message + "\nWriting Geopolygon to Image... (" + QString::number(progress_percent) + "%)");
		}
	}
}

/**
**	@brief	Set the province's terrain
**
**	@param	terrain	The new terrain
*/
void province::set_terrain(metternich::terrain_type *terrain)
{
	if (terrain == this->get_terrain()) {
		return;
	}

	const metternich::terrain_type *old_terrain = this->get_terrain();

	if (old_terrain != nullptr) {
		if (old_terrain->get_modifier() != nullptr) {
			old_terrain->get_modifier()->remove(this);
		}

		if (old_terrain->is_river()) {
			province::river_provinces.erase(this);
		}
	}

	this->terrain = terrain;

	if (terrain != nullptr) {
		if (terrain->get_modifier() != nullptr) {
			terrain->get_modifier()->Apply(this);
		}

		if (terrain->is_river()) {
			province::river_provinces.insert(this);
		}
	}

	emit terrain_changed();
}

/**
**	@brief	Set the province's population
**
**	@param	population	The new population size for the province
*/
void province::set_population(const int population)
{
	if (population == this->get_population()) {
		return;
	}

	this->population = population;
	emit population_changed();
}

/**
**	@brief	Calculate the population size for the province
*/
void province::calculate_population()
{
	int population = 0;
	for (const holding *holding : this->get_holdings()) {
		population += holding->get_population();
	}
	this->set_population(population);
}

/**
**	@brief	Set the province's population capacity additive modifier
**
**	@param	population	The new population capacity additive modifier for the province
*/
void province::set_population_capacity_additive_modifier(const int population_capacity_modifier)
{
	if (population_capacity_modifier == this->get_population_capacity_additive_modifier()) {
		return;
	}

	for (holding *holding : this->get_holdings()) {
		holding->change_base_population_capacity(-this->get_population_capacity_additive_modifier());
	}

	this->population_capacity_additive_modifier = population_capacity_modifier;

	for (holding *holding : this->get_holdings()) {
		holding->change_base_population_capacity(this->get_population_capacity_additive_modifier());
	}
}

/**
**	@brief	Set the province's population capacity modifier
**
**	@param	population	The new population capacity modifier for the province
*/
void province::set_population_capacity_modifier(const int population_capacity_modifier)
{
	if (population_capacity_modifier == this->get_population_capacity_modifier()) {
		return;
	}

	for (holding *holding : this->get_holdings()) {
		holding->change_population_capacity_modifier(-this->get_population_capacity_modifier());
	}

	this->population_capacity_modifier = population_capacity_modifier;

	for (holding *holding : this->get_holdings()) {
		holding->change_population_capacity_modifier(this->get_population_capacity_modifier());
	}
}

/**
**	@brief	Set the province's population growth modifier
**
**	@param	population	The new population growth modifier for the province
*/
void province::set_population_growth_modifier(const int population_growth_modifier)
{
	if (population_growth_modifier == this->get_population_growth_modifier()) {
		return;
	}

	for (holding *holding : this->get_holdings()) {
		holding->change_base_population_growth(-this->get_population_growth_modifier());
	}

	this->population_growth_modifier = population_growth_modifier;

	for (holding *holding : this->get_holdings()) {
		holding->change_base_population_growth(this->get_population_growth_modifier());
	}
}

/**
**	@brief	Calculate the population for each culture, religion and etc.
*/
void province::calculate_population_groups()
{
	this->population_per_type.clear();
	this->population_per_culture.clear();
	this->population_per_religion.clear();

	for (holding *holding : this->get_holdings()) {
		for (const auto &kv_pair : holding->get_population_per_type()) {
			this->population_per_type[kv_pair.first] += kv_pair.second;
		}
		for (const auto &kv_pair : holding->get_population_per_culture()) {
			this->population_per_culture[kv_pair.first] += kv_pair.second;
		}
		for (const auto &kv_pair : holding->get_population_per_religion()) {
			this->population_per_religion[kv_pair.first] += kv_pair.second;
		}
	}

	emit population_groups_changed();

	//update the province's main culture and religion

	metternich::culture *plurality_culture = nullptr;
	int plurality_culture_size = 0;

	for (const auto &kv_pair : this->population_per_culture) {
		metternich::culture *culture = kv_pair.first;
		const int culture_size = kv_pair.second;
		if (plurality_culture == nullptr || culture_size > plurality_culture_size) {
			plurality_culture = culture;
			plurality_culture_size = culture_size;
		}
	}

	metternich::religion *plurality_religion = nullptr;
	int plurality_religion_size = 0;

	for (const auto &kv_pair : this->population_per_religion) {
		metternich::religion *religion = kv_pair.first;
		const int religion_size = kv_pair.second;
		if (plurality_religion == nullptr || religion_size > plurality_religion_size) {
			plurality_religion = religion;
			plurality_religion_size = religion_size;
		}
	}

	this->set_culture(plurality_culture);
	this->set_religion(plurality_religion);
}

/**
**	@brief	Get the province's holdings
*/
QVariantList province::get_holdings_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_holdings());
}

/**
**	@brief	Get the province's holding slots
*/
QVariantList province::get_holding_slots_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_holding_slots());
}

/**
**	@brief	Get one of the province's holdings
**
**	@param	holding_slot	The holding's slot
*/
holding *province::get_holding(holding_slot *holding_slot) const
{
	auto find_iterator = this->holdings_by_slot.find(holding_slot);
	if (find_iterator != this->holdings_by_slot.end()) {
		return find_iterator->second.get();
	}

	return nullptr;
}

/**
**	@brief	Create a holding in the province
**
**	@param	holding_slot	The holding's slot
**
**	@param	type			The holding's type
*/
void province::create_holding(holding_slot *holding_slot, holding_type *type)
{
	auto new_holding = std::make_unique<holding>(holding_slot, type);
	new_holding->moveToThread(QApplication::instance()->thread());
	this->holdings.push_back(new_holding.get());
	this->holdings_by_slot.insert({holding_slot, std::move(new_holding)});
	emit holdings_changed();
	if (this->get_capital_holding() == nullptr) {
		this->set_capital_holding(this->holdings.front());
	}
}

/**
**	@brief	Destroy a holding in the province
**
**	@param	holding_slot	The holding's slot
*/
void province::destroy_holding(holding_slot *holding_slot)
{
	holding *holding = this->get_holding(holding_slot);
	if (holding == this->get_capital_holding()) {
		//if the capital holding is being destroyed, set the next holding as the capital, if any exists, or otherwise set the capital holding to null
		if (this->holdings.size() > 1) {
			this->set_capital_holding(this->holdings.at(1));
		} else {
			this->set_capital_holding(nullptr);
		}
	}
	this->holdings.erase(std::remove(this->holdings.begin(), this->holdings.end(), holding), this->holdings.end());
	this->holdings_by_slot.erase(holding_slot);
	emit holdings_changed();
}

/**
**	@brief	Add a population unit to the province
*/
void province::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

/**
**	@brief	Get whether this province borders a water province
**
**	@return	True if the province borders a water province, or false otherwise
*/
bool province::borders_water() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->is_water()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether this province borders a river province
**
**	@return	True if the province borders a river province, or false otherwise
*/
bool province::borders_river() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->is_river()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether the province has a river, that is, whether it either borders a major river, or contains a minor river within itself
**
**	@return	True if the province has a river, or false otherwise
*/
bool province::has_river() const
{
	return this->has_inner_river() || this->borders_river();
}

/**
**	@brief	Get whether this province is coastal
**
**	@return	True if the province is coastal, or false otherwise
*/
bool province::is_coastal() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->is_ocean()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether this province is a water province
**
**	@return	True if the province is a water province, or false otherwise
*/
bool province::is_water() const
{
	return this->get_terrain() != nullptr && this->get_terrain()->is_water();
}

/**
**	@brief	Get whether this province is an ocean province
**
**	@return	True if the province is an ocean province, or false otherwise
*/
bool province::is_ocean() const
{
	return this->get_terrain() != nullptr && this->get_terrain()->is_ocean();
}

/**
**	@brief	Get whether this province is a river province
**
**	@return	True if the province is a river province, or false otherwise
*/
bool province::is_river() const
{
	return this->get_terrain() != nullptr && this->get_terrain()->is_river();
}

/**
**	@brief	Set whether the province is selected
**
**	@param	selected				Whether the province is being selected
**
**	@param	notify_engine_interface	Whether to emit a signal notifying the engine interface of the change
*/
void province::set_selected(const bool selected, const bool notify_engine_interface)
{
	if (selected == this->is_selected()) {
		return;
	}

	if (selected) {
		if (province::selected_province != nullptr) {
			province::selected_province->set_selected(false, false);
		}
		province::selected_province = this;
	} else {
		province::selected_province = nullptr;
	}

	this->selected = selected;
	emit selected_changed();

	if (notify_engine_interface) {
		EngineInterface::get()->emit selected_province_changed();
	}
}

/**
**	@brief	Get whether the province is selectable
**
**	@return	True if the province is selectable, or false otherwise
*/
bool province::is_selectable() const
{
	return this->get_county() != nullptr;
}

QVariantList province::get_population_per_type_qvariant_list() const
{
	QVariantList population_per_type;

	for (const auto &kv_pair : this->population_per_type) {
		QVariantMap type_population;
		type_population["type"] = QVariant::fromValue(kv_pair.first);
		type_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_type.append(type_population);
	}

	return population_per_type;
}

QVariantList province::get_population_per_culture_qvariant_list() const
{
	QVariantList population_per_culture;

	for (const auto &kv_pair : this->population_per_culture) {
		QVariantMap culture_population;
		culture_population["culture"] = QVariant::fromValue(kv_pair.first);
		culture_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_culture.append(culture_population);
	}

	return population_per_culture;
}

QVariantList province::get_population_per_religion_qvariant_list() const
{
	QVariantList population_per_religion;

	for (const auto &kv_pair : this->population_per_religion) {
		QVariantMap religion_population;
		religion_population["religion"] = QVariant::fromValue(kv_pair.first);
		religion_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_religion.append(religion_population);
	}

	return population_per_religion;
}

QVariantList province::get_geopolygons_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_geopolygons());
}

QVariantList province::get_geopaths_qvariant_list() const
{
	return util::container_to_qvariant_list(this->geopaths);
}

QGeoCoordinate province::get_center_coordinate() const
{
	return map::get()->get_pixel_pos_coordinate(this->rect.center());
}

}
