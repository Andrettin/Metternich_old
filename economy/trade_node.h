#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <set>

namespace metternich {

class province;
class world;

class trade_node final : public data_entry, public data_type<trade_node>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(metternich::province* center_of_trade READ get_center_of_trade WRITE set_center_of_trade NOTIFY center_of_trade_changed)
	Q_PROPERTY(bool active READ is_active NOTIFY active_changed)

public:
	trade_node(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "trade_node";
	static constexpr const char *database_folder = "trade_nodes";

private:
	static inline const std::vector<province *> empty_path;

public:
	virtual void initialize() override;
	virtual void check() const override;

	virtual void initialize_history() override
	{
		this->calculate_trade_paths();
	}
	
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

	world *get_world() const;

	const std::vector<province *> &get_trade_path(const trade_node *other_node) const
	{
		auto find_iterator = this->trade_paths.find(other_node);
		if (find_iterator != this->trade_paths.end()) {
			return find_iterator->second;
		}

		return trade_node::empty_path;
	}

	void set_trade_path(const trade_node *other_node, const std::vector<province *> &path, const bool notify = true);
	void calculate_trade_paths();
	void calculate_trade_path(const trade_node *other_node, const bool notify = true);
	void clear_trade_paths();

signals:
	void center_of_trade_changed();
	void active_changed();
	void trade_paths_changed();

private:
	QColor color;
	province *center_of_trade = nullptr;
	bool active = false; //whether this trade node is active, i.e. whether its center of trade has an owner
	std::set<province *> provinces;
	std::map<const trade_node *, int> trade_costs; //trade costs with other trade nodes
	std::map<const trade_node *, std::vector<province *>> trade_paths; //paths to other trade nodes
};

}
