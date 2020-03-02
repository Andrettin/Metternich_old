#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QGeoCoordinate>
#include <QPoint>

#include <string>
#include <vector>

namespace metternich {

class commodity;
class holding;
class landed_title;
class province;
class province_profile;
class terrain_type;
class territory;
class world;
enum class holding_slot_type : int;

class holding_slot : public data_entry, public data_type<holding_slot>
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring NOTIFY name_changed)
	Q_PROPERTY(metternich::territory* territory READ get_territory CONSTANT)
	Q_PROPERTY(metternich::province* province READ get_province WRITE set_province)
	Q_PROPERTY(metternich::province_profile* province_profile MEMBER province_profile)
	Q_PROPERTY(metternich::world* world READ get_world WRITE set_world)
	Q_PROPERTY(metternich::holding_slot_type type READ get_type WRITE set_type)
	Q_PROPERTY(bool settlement READ is_settlement CONSTANT)
	Q_PROPERTY(metternich::landed_title* barony READ get_barony WRITE set_barony NOTIFY barony_changed)
	Q_PROPERTY(metternich::holding* holding READ get_holding NOTIFY holding_changed)
	Q_PROPERTY(bool megalopolis READ is_megalopolis CONSTANT)
	Q_PROPERTY(metternich::terrain_type* terrain READ get_terrain WRITE set_terrain NOTIFY terrain_changed)
	Q_PROPERTY(QGeoCoordinate geocoordinate READ get_geocoordinate WRITE set_geocoordinate)
	Q_PROPERTY(QPoint pos READ get_pos WRITE set_pos NOTIFY pos_changed)
	Q_PROPERTY(int holding_size READ get_holding_size WRITE set_holding_size NOTIFY holding_size_changed)
	Q_PROPERTY(QVariantList available_commodities READ get_available_commodities_qvariant_list NOTIFY available_commodities_changed)
	Q_PROPERTY(bool population_distribution_allowed MEMBER population_distribution_allowed READ is_population_distribution_allowed)

public:
	static constexpr const char *class_identifier = "holding_slot";
	static constexpr const char *database_folder = "holding_slots";
	static constexpr const char *prefix = "h_";

	static std::set<std::string> get_database_dependencies();

	static holding_slot *add(const std::string &identifier)
	{
		if (identifier.substr(0, 2) != holding_slot::prefix) {
			throw std::runtime_error("Invalid identifier for new holding slot: \"" + identifier + "\". Holding slot identifiers must begin with \"" + holding_slot::prefix + "\".");
		}

		holding_slot *holding_slot = data_type<metternich::holding_slot>::add(identifier);
		return holding_slot;
	}

	static void process_geojson_feature(const QVariantMap &feature)
	{
		const QVariantMap properties = feature.value("properties").toMap();
		const QString holding_slot_identifier = properties.value("holding_slot").toString();

		holding_slot *slot = holding_slot::get(holding_slot_identifier.toStdString());

		const QGeoCircle geocircle = feature.value("data").value<QGeoCircle>();
		slot->set_geocoordinate(geocircle.center());
	}

	holding_slot(const std::string &identifier);
	virtual ~holding_slot() override;

	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;
	virtual void check_history() const override;
	virtual gsml_data get_cache_data() const override;

	virtual std::string get_name() const override;

	QString get_name_qstring() const
	{
		return QString::fromStdString(this->get_name());
	}

	std::vector<std::vector<std::string>> get_tag_suffix_list_with_fallbacks() const;

	holding_slot_type get_type() const
	{
		return this->type;
	}

	void set_type(const holding_slot_type type)
	{
		if (type == this->get_type()) {
			return;
		}

		if (this->get_territory() != nullptr) {
			throw std::runtime_error("Tried to change the type of holding slot \"" + this->get_identifier() + "\" after it had already been added to a territory.");
		}

		this->type = type;
	}

	bool is_settlement() const;

	landed_title *get_barony() const
	{
		return this->barony;
	}

	void set_barony(landed_title *barony);

	holding *get_holding() const
	{
		return this->holding.get();
	}

	void set_holding(qunique_ptr<holding> &&holding);

	territory *get_territory() const;

	province *get_province() const
	{
		return this->province;
	}

	void set_province(province *province);

	world *get_world() const
	{
		return this->world;
	}

	void set_world(world *world);

	bool is_megalopolis() const
	{
		return this->get_world() != nullptr;
	}

	terrain_type *get_terrain() const
	{
		return this->terrain;
	}

	void set_terrain(terrain_type *terrain);

	const QPoint &get_pos() const
	{
		return this->pos;
	}

	void set_pos(const QPoint &pos)
	{
		if (pos == this->get_pos()) {
			return;
		}

		this->pos = pos;
		emit pos_changed();
	}

	const QGeoCoordinate &get_geocoordinate() const
	{
		return this->geocoordinate;
	}

	void set_geocoordinate(const QGeoCoordinate &geocoordinate)
	{
		this->geocoordinate = geocoordinate;
	}

	int get_holding_size() const
	{
		return this->holding_size;
	}

	void set_holding_size(const int size)
	{
		if (size == this->get_holding_size()) {
			return;
		}

		this->holding_size = size;
		emit holding_size_changed();
	}

	bool is_territory_capital() const;

	const std::vector<commodity *> &get_available_commodities() const
	{
		return this->available_commodities;
	}

	QVariantList get_available_commodities_qvariant_list() const;

	Q_INVOKABLE void add_available_commodity(commodity *commodity)
	{
		this->available_commodities.push_back(commodity);
		emit available_commodities_changed();
	}

	Q_INVOKABLE void remove_available_commodity(commodity *commodity)
	{
		this->available_commodities.erase(std::remove(this->available_commodities.begin(), this->available_commodities.end(), commodity), this->available_commodities.end());
		emit available_commodities_changed();
	}

	void generate_available_commodity();

	bool is_population_distribution_allowed() const
	{
		return this->population_distribution_allowed;
	}

	bool has_any_trade_route() const;
	bool has_any_active_trade_route() const;
	bool has_any_trade_route_land_connection() const;

	const std::vector<metternich::province *> &get_megalopolis_provinces() const
	{
		return this->megalopolis_provinces;
	}

	void add_megalopolis_province(province *province)
	{
		this->megalopolis_provinces.push_back(province);
	}

	void remove_megalopolis_province(province *province);
	void amalgamate_megalopolis();
	void create_holding_from_megalopolis_provinces();
	void move_population_from_megalopolis_provinces();

signals:
	void barony_changed();
	void holding_changed();
	void terrain_changed();
	void pos_changed();
	void holding_size_changed();
	void available_commodities_changed();
	void active_trade_routes_changed();

private:
	holding_slot_type type; //the type of the holding slot
	landed_title *barony = nullptr; //the barony corresponding to this holding slot
	qunique_ptr<holding> holding; //the holding built on this slot, if any
	province *province = nullptr; //to which province this holding slot belongs
	province_profile *province_profile = nullptr;
	world *world = nullptr; //to which world this holding slot belongs, if it is a world holding slot
	terrain_type *terrain = nullptr;
	QGeoCoordinate geocoordinate;
	QPoint pos = QPoint(-1, -1);
	int holding_size = 100; //the holding size, which affects population capacity (100 = normal size)
	std::vector<metternich::commodity *> available_commodities; //the commodities available for production by the holding (if any)
	bool population_distribution_allowed = true;
	std::vector<metternich::province *> megalopolis_provinces; //provinces which pertain to this megalopolis holding slot, if it is one
};

}
