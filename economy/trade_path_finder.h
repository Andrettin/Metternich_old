#pragma once

namespace metternich {

class province;

//finds the best trade path between two provinces
class trade_path_finder
{
public:
	bool find_path(const province *start_province, const province *goal_province);

	int get_trade_cost() const
	{
		return this->trade_cost;
	}

	std::vector<province *> release_path()
	{
		return std::move(this->path);
	}

private:
	int trade_cost = 0; //the resulting trade cost
	std::vector<province *> path; //the resulting trade path
};

}
