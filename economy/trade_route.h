#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QRect>

namespace metternich {

class province;
class world;

class trade_route final : public data_entry, public data_type<trade_route>
{
	class path_element;

	Q_OBJECT

	Q_PROPERTY(QVariantList path READ get_path_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList path_branch_points READ get_path_branch_points_qvariant_list NOTIFY path_branch_points_changed)
	Q_PROPERTY(QString path_branch_points_svg READ get_path_branch_points_svg NOTIFY path_branch_points_changed)
	Q_PROPERTY(QRect rect MEMBER rect READ get_rect CONSTANT)
	Q_PROPERTY(bool active READ is_active NOTIFY active_changed)

public:
	static constexpr const char *class_identifier = "trade_route";
	static constexpr const char *database_folder = "trade_routes";

	static void add_path_points_to_qvariantlist(QVariantList &point_list, const std::vector<const province *> &path_provinces, const QPoint &start_map_pos);

	trade_route(const std::string &identifier);
	virtual ~trade_route() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;

	world *get_world() const
	{
		return this->world;
	}

	void set_world(world *world);

	QVariantList get_path_qvariant_list() const;

	void add_path_branch(const std::vector<province *> &path_branch);
	void clear_path();

	QVariantList get_path_branch_points_qvariant_list() const;
	QString get_path_branch_points_svg() const;

	bool is_endpoint(const province *province) const
	{
		return this->path_endpoints.contains(province);
	}

	bool has_connection_between(const province *source_province, const province *target_province) const;
	bool has_any_land_connection_for_province(const province *province) const;

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

private:
	path_element *get_province_path_element(const province *province) const
	{
		auto find_iterator = this->path.find(province);

		if (find_iterator != this->path.end()) {
			return find_iterator->second.get();
		}

		throw std::runtime_error("No path element in trade route \"" + this->get_identifier() + "\" found for province.");
	}

signals:
	void active_changed();
	void path_branch_points_changed();

private:
	world *world = nullptr;
	std::set<province *> provinces; //the provinces through which the trade route passes
	std::map<const province *, std::unique_ptr<path_element>> path;
	std::set<const province *> path_endpoints;
	std::vector<std::vector<const province *>> path_branch_provinces; //used to draw the trade route
	QRect rect;
	bool active = false; //whether the trade route is active
};

}
