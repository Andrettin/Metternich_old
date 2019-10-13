#include "map/province.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding.h"
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

#include <QApplication>
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
	typename std::map<QRgb, province *>::const_iterator find_iterator = province::instances_by_rgb.find(rgb);

	if (find_iterator != province::instances_by_rgb.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		throw std::runtime_error("No province found for RGB value: " + std::to_string(rgb) + ".");
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
	if (property.get_key().substr(0, 2) == LandedTitle::BaronyPrefix) {
		//a property related to one of the province's holdings
		LandedTitle *barony = LandedTitle::get(property.get_key());
		holding *holding = this->get_holding(barony);
		if (property.get_operator() == gsml_operator::assignment) {
			//the assignment operator sets the holding's type (creating the holding if it doesn't exist)
			holding_type *holding_type = holding_type::get(property.get_value());
			if (holding != nullptr) {
				holding->set_type(holding_type);
			} else {
				this->create_holding(barony, holding_type);
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
			this->geopaths.push_back(path_data.to_geopath());
		}
	} else if (tag == "border_provinces") {
		for (const std::string &border_province_identifier : scope.get_values()) {
			province *border_province = province::get(border_province_identifier);
			this->border_provinces.insert(border_province);
		}
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

	if (tag.substr(0, 2) == LandedTitle::BaronyPrefix) {
		//a change to the data of one of the province's holdings

		LandedTitle *barony = LandedTitle::get(tag);
		holding *holding = this->get_holding(barony);
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

	if (this->borders_river()) {
		this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province borders a river
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
	cache_data.add_property("terrain", this->get_terrain()->get_identifier());

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
void province::set_county(LandedTitle *county)
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
LandedTitle *province::get_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->GetDeJureLiegeTitle();
	}

	return nullptr;
}

/**
**	@brief	Get the province's (de jure) kingdom
**
**	@return	The province's (de jure) kingdom
*/
LandedTitle *province::get_kingdom() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->GetDeJureLiegeTitle();
	}

	return nullptr;
}

/**
**	@brief	Get the province's (de jure) empire
**
**	@return	The province's (de jure) empire
*/
LandedTitle *province::get_empire() const
{
	if (this->get_kingdom() != nullptr) {
		return this->get_kingdom()->GetDeJureLiegeTitle();
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
		QPoint pixel_pos = map::get()->get_pixel_position(index);
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
		QPoint pixel_pos = map::get()->get_pixel_position(index) - this->rect.topLeft();
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
		QPoint pixel_pos = map::get()->get_pixel_position(index) - this->rect.topLeft();
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
		const LandedTitle *realm = this->get_county()->GetRealm();
		if (realm != nullptr) {
			province_color = realm->get_color();
		} else {
			province_color = this->get_county()->get_color();
		}
	} else if (this->get_terrain()->is_water()) {
		province_color = QColor("#0080ff");
	} else {
		province_color = QColor(Qt::darkGray); //wasteland
	}

	QColor border_color;
	if (this->is_selected()) {
		//if the province is selected, highlight its border pixels
		border_color = QColor(Qt::yellow);
	} else {
		border_color = QColor(province_color.red() / 2, province_color.green() / 2, province_color.blue() / 2);
	}

	this->image.setColor(1, province_color.rgb());
	this->image.setColor(2, border_color.rgb());

	emit image_changed();
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

	if (old_terrain != nullptr && old_terrain->get_modifier() != nullptr) {
		old_terrain->get_modifier()->remove(this);
	}

	this->terrain = terrain;

	if (terrain != nullptr && terrain->get_modifier() != nullptr) {
		terrain->get_modifier()->Apply(this);
	}

	emit terrain_changed();
}

/**
**	@brief	Calculate the province's terrain
*/
void province::calculate_terrain()
{
	std::map<terrain_type *, int> terrain_counts;

	for (const QGeoPolygon &geopolygon : this->geopolygons) {
		std::vector<QGeoCoordinate> checked_coordinates;

		QGeoCoordinate center_coordinate = geopolygon.center();
		checked_coordinates.push_back(center_coordinate);

		for (const QGeoCoordinate &border_coordinate : geopolygon.path()) {
			const double latitude = (center_coordinate.latitude() + border_coordinate.latitude()) / 2;
			const double longitude = (center_coordinate.longitude() + border_coordinate.longitude()) / 2;
			checked_coordinates.emplace_back(latitude, longitude);
		}

		for (const QGeoCoordinate &coordinate : checked_coordinates) {
			if (!geopolygon.contains(coordinate)) {
				continue;
			}

			terrain_type *terrain = map::get()->get_coordinate_terrain(coordinate);

			if (terrain != nullptr) {
				terrain_counts[terrain]++;
			}
		}
	}

	terrain_type *best_terrain = nullptr;
	int best_terrain_count = 0;
	for (const auto &kv_pair : terrain_counts) {
		terrain_type *terrain = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_terrain_count) {
			best_terrain = terrain;
			best_terrain_count = count;
		}
	}

	if (best_terrain != nullptr) {
		this->set_terrain(best_terrain);
	} else {
		throw std::runtime_error("No terrain could be calculated for province \"" + this->get_identifier() + "\".");
	}
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
**	@brief	Get one of the province's holdings
**
**	@param	barony	The holding's barony
*/
holding *province::get_holding(LandedTitle *barony) const
{
	auto find_iterator = this->holdings_by_barony.find(barony);
	if (find_iterator != this->holdings_by_barony.end()) {
		return find_iterator->second.get();
	}

	return nullptr;
}

/**
**	@brief	Create a holding in the province
**
**	@param	barony	The holding's barony
**
**	@param	type	The holding's type
*/
void province::create_holding(LandedTitle *barony, holding_type *type)
{
	auto new_holding = std::make_unique<holding>(barony, type, this);
	new_holding->moveToThread(QApplication::instance()->thread());
	this->holdings.push_back(new_holding.get());
	this->holdings_by_barony.insert({barony, std::move(new_holding)});
	emit holdings_changed();
	if (this->get_capital_holding() == nullptr) {
		this->set_capital_holding(this->holdings.front());
	}

	if (Game::get()->IsRunning()) {
		if (new_holding->get_commodity() == nullptr) {
			new_holding->generate_commodity();
		}
	}
}

/**
**	@brief	Destroy a holding in the province
**
**	@param	barony	The holding's barony
*/
void province::destroy_holding(LandedTitle *barony)
{
	holding *holding = this->get_holding(barony);
	if (holding == this->get_capital_holding()) {
		//if the capital holding is being destroyed, set the next holding as the capital, if any exists, or otherwise set the capital holding to null
		if (this->holdings.size() > 1) {
			this->set_capital_holding(this->holdings.at(1));
		} else {
			this->set_capital_holding(nullptr);
		}
	}
	this->holdings.erase(std::remove(this->holdings.begin(), this->holdings.end(), holding), this->holdings.end());
	this->holdings_by_barony.erase(barony);
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
**	@brief	Calculate the province's border provinces
*/
void province::calculate_border_provinces()
{
	std::set<province *> border_provinces;
	std::vector<QGeoCoordinate> border_coordinates;

	for (const QGeoPolygon &geopolygon : this->geopolygons) {
		QList<QGeoCoordinate> coordinates = geopolygon.path();

		for (int i = 0; i < geopolygon.holesCount(); ++i) {
			coordinates.append(geopolygon.holePath(i));
		}

		for (const QGeoCoordinate &coordinate : coordinates) {
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 0));
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 90));
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 180));
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 270));
		}
	}

	for (const QGeoPath &geopath : this->geopaths) {
		QList<QGeoCoordinate> coordinates = geopath.path();

		for (const QGeoCoordinate &coordinate : coordinates) {
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 0));
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 90));
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 180));
			border_coordinates.push_back(coordinate.atDistanceAndAzimuth(1000, 270));
		}
	}

	for (const QGeoCoordinate &coordinate : border_coordinates) {
		province *coordinate_province = map::get()->get_coordinate_province(coordinate);
		if (coordinate_province != nullptr && coordinate_province != this) {
			border_provinces.insert(coordinate_province);
			coordinate_province->border_provinces.insert(this);
		}
	}
}

/**
**	@brief	Get whether this province borders a water province
**
**	@return	True if the province borders a water province, or false otherwise
*/
bool province::borders_water() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->get_terrain()->is_water()) {
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
		if (border_province->get_terrain()->is_river()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether this province is coastal
**
**	@return	True if the province is coastal, or false otherwise
*/
bool province::is_coastal() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->get_terrain()->is_ocean()) {
			return true;
		}
	}

	return false;
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

}
