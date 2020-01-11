#pragma once

#include <memory>
#include <set>

namespace metternich {

class pathfinder_implementation;
class province;

struct find_trade_path_result
{
	find_trade_path_result(const bool success) : success(success)
	{}

	bool success = false;
	int trade_cost = 0; //the resulting trade cost
	std::vector<province *> path; //the resulting trade path
};

class pathfinder
{
	class impl;

public:
	pathfinder(const std::set<province *> &provinces);
	~pathfinder();

	find_trade_path_result find_trade_path(const province *start_province, const province *goal_province) const;

private:
	std::unique_ptr<impl> implementation;
};

}
