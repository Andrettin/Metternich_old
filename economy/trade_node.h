#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

namespace metternich {

class province;
class trade_route;

class trade_node : public data_entry, public data_type<trade_node>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(metternich::province* center_of_trade MEMBER center_of_trade READ get_center_of_trade NOTIFY center_of_trade_changed)

public:
	trade_node(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "trade_node";
	static constexpr const char *database_folder = "trade_nodes";

	virtual void check() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

	province *get_center_of_trade() const
	{
		return this->center_of_trade;
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

signals:
	void center_of_trade_changed();

private:
	QColor color;
	province *center_of_trade = nullptr;
	std::set<province *> provinces;
	std::set<trade_route *> trade_routes; //the trade routes going through the trade node
};

}
