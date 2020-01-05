#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <set>

namespace metternich {

class province;
class trade_route;
class world;

class trade_node : public data_entry, public data_type<trade_node>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(metternich::province* center_of_trade READ get_center_of_trade WRITE set_center_of_trade NOTIFY center_of_trade_changed)
	Q_PROPERTY(metternich::trade_node* trade_area READ get_trade_area NOTIFY trade_area_changed)

public:
	trade_node(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "trade_node";
	static constexpr const char *database_folder = "trade_nodes";

	static const std::set<trade_node *> &get_all_active()
	{
		return trade_node::active_trade_nodes;
	}

private:
	static inline std::set<trade_node *> active_trade_nodes;
	static inline std::set<trade_node *> major_trade_nodes;

public:
	virtual void check() const override;
	
	virtual std::string get_name() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

	province *get_center_of_trade() const
	{
		return this->center_of_trade;
	}

	void set_center_of_trade(province *province);

	bool is_active() const
	{
		return this->active;
	}

	void set_active(const bool active);

	bool is_major() const
	{
		return this->major;
	}

	void set_major(const bool major);

	trade_node *get_trade_area() const
	{
		return this->trade_area;
	}

	void set_trade_area(trade_node *trade_area);
	void calculate_trade_area();

	const std::set<province *> &get_provinces() const
	{
		return this->provinces;
	}

	void add_province(province *province)
	{
		this->provinces.insert(province);
	}

	void remove_province(province *province)
	{
		this->provinces.erase(province);
	}

	void add_trade_route(trade_route *route)
	{
		this->trade_routes.insert(route);
	}

	const std::set<trade_node *> &get_trade_nodes() const
	{
		return this->trade_nodes;
	}

	world *get_world() const;

signals:
	void center_of_trade_changed();
	void trade_area_changed();

private:
	QColor color;
	province *center_of_trade = nullptr;
	bool active = false; //whether this trade node is active, i.e. whether its center of trade has an owner
	bool major = false; //whether this trade node is a major one, i.e. whether it is also a trade area
	trade_node *trade_area = nullptr;
	std::set<province *> provinces;
	std::set<trade_route *> trade_routes; //the trade routes going through the trade node
	std::set<trade_node *> trade_nodes; //the trade (sub-)nodes, if this is a trade area
};

}
