#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QRect>

namespace metternich {

class province;
class world;

class trade_route_path_element
{
public:
	trade_route_path_element(province *province) : province(province)
	{
	}

	province *get_province() const
	{
		return this->province;
	}

	const std::set<trade_route_path_element *> &get_previous() const
	{
		return this->previous;
	}

	void add_to_previous(trade_route_path_element *element)
	{
		this->previous.insert(element);

		if (!element->next.contains(this)) {
			element->next.insert(this);
		}
	}

	const std::set<trade_route_path_element *> &get_next() const
	{
		return this->next;
	}

private:
	province *province = nullptr; //the province for this path element
	std::set<trade_route_path_element *> previous; //path elements immediately preceding this one
	std::set<trade_route_path_element *> next; //path elements immediately succeeding this one
};

class trade_route : public data_entry, public data_type<trade_route>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList path READ get_path_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList path_branch_points READ get_path_branch_points_qvariant_list CONSTANT)
	Q_PROPERTY(QRect rect MEMBER rect READ get_rect CONSTANT)
	Q_PROPERTY(bool active READ is_active NOTIFY active_changed)

public:
	trade_route(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "trade_route";
	static constexpr const char *database_folder = "trade_routes";

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

	bool is_endpoint(const province *province) const
	{
		return this->path_endpoints.contains(province);
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
	void path_branch_points_changed();

private:
	world *world = nullptr;
	std::map<province *, std::unique_ptr<trade_route_path_element>> path;
	std::set<const province *> path_endpoints;
	std::vector<std::vector<const province *>> path_branch_provinces; //used to draw the trade route
	QRect rect;
	bool active = false; //whether the trade route is active
};

}
