#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QRect>

namespace metternich {

class province;
class trade_node;

class trade_route : public data_entry, public data_type<trade_route>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList path READ get_path_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList path_points READ get_path_points_qvariant_list CONSTANT)
	Q_PROPERTY(QRect rect MEMBER rect READ get_rect)

public:
	trade_route(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "trade_route";
	static constexpr const char *database_folder = "trade_routes";

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;
	
	void add_trade_node(trade_node *node);

	QVariantList get_path_qvariant_list() const;
	QVariantList get_path_points_qvariant_list() const;

	const QRect &get_rect() const
	{
		return this->rect;
	}

private:
	std::vector<province *> path;
	std::set<trade_node *> trade_nodes;
	QRect rect;
};

}
