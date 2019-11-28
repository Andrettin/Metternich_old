#pragma once

namespace metternich {

class condition_check_base
{
public:
	static void recalculate_pending_checks()
	{
		for (condition_check_base *check : condition_check_base::checks_to_recalculate) {
			check->calculate_result();
			check->result_recalculation_needed = false;
		}

		condition_check_base::checks_to_recalculate.clear();
	}

private:
	static inline std::set<condition_check_base *> checks_to_recalculate;

protected:
	condition_check_base(const std::function<void(bool)> &result_setter) : result_setter(result_setter)
	{
	}

	virtual ~condition_check_base() {}

public:
	void set_result(const bool result)
	{
		this->result_setter(result);
	}

	void set_result_recalculation_needed()
	{
		if (this->result_recalculation_needed) {
			return;
		}

		this->result_recalculation_needed = true;
		condition_check_base::checks_to_recalculate.insert(this);
	}

	virtual void calculate_result() = 0;

private:
	std::function<void(bool)> result_setter; //setter for the result
	bool result_recalculation_needed = false;
};

}
