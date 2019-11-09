#pragma once

#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

class condition;

/**
**	@brief	A check for a scripted condition
*/
template <typename T>
class condition_check : public condition_check_base
{
public:
	condition_check(const metternich::condition *condition, const T *checked_instance, const std::function<void(bool)> &result_setter)
		: condition_check_base(condition, result_setter), checked_instance(checked_instance)
	{
		if (this->get_condition() != nullptr) {
			this->get_condition()->bind_condition_check(*this, this->checked_instance);
		}

		this->calculate_result(); //calculate initial result
	}

	virtual ~condition_check() override {}

	virtual void calculate_result() override
	{
		if (this->get_condition() != nullptr) {
			this->set_result(this->get_condition()->check(this->checked_instance));
		} else {
			this->set_result(true); //always true if there are no conditions
		}
	}

private:
	const T *checked_instance = nullptr;
};

}
