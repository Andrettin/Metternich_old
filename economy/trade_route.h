#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QGeoPath>
#include <QRect>

namespace metternich {

class province;
class world;

class trade_route : public data_entry, public data_type<trade_route>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList path READ get_path_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList path_points READ get_path_points_qvariant_list CONSTANT)
	Q_PROPERTY(QRect rect MEMBER rect READ get_rect CONSTANT)
	Q_PROPERTY(bool active READ is_active NOTIFY active_changed)

public:
	trade_route(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "trade_route";
	static constexpr const char *database_folder = "trade_routes";

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;
	virtual gsml_data get_cache_data() const override;

	const QGeoPath &get_geopath() const
	{
		return this->geopath;
	}

	void set_geopath(const QGeoPath &geopath);
	void calculate_path_from_geopath();

	world *get_world() const
	{
		return this->world;
	}

	void set_world(world *world);

	QVariantList get_path_qvariant_list() const;

	void add_path_province(province *path_province);
	void clear_path();

	QVariantList get_path_points_qvariant_list() const;

	bool is_endpoint(const province *province) const
	{
		return path.front() == province || path.back() == province;
	}

	const QRect &get_rect() const
	{
		return this->rect;
	}

	bool is_active() const
	{
		return this->active;
	}

	void set_active(const bool active);
	void calculate_active();

signals:
	void active_changed();

private:
	world *world = nullptr;
	std::vector<province *> path;
	QRect rect;
	QGeoPath geopath;
	bool active = false; //whether the trade route is active
};

}
