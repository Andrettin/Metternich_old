#pragma once

#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"
#include "script/context.h"

namespace metternich {

template <typename T>
class condition;

/**
**	@brief	A check for a scripted condition
*/
template <typename T>
class condition_check final : public condition_check_base
{
public:
	condition_check(const metternich::condition<T> *condition, const T *checked_instance, const std::function<void(bool)> &result_setter)
		: condition_check_base(result_setter), condition(condition), checked_instance(checked_instance)
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
			read_only_context ctx;

			if constexpr (std::is_same_v<T, character>) {
				ctx.current_character = this->checked_instance;
			}

			this->set_result(this->get_condition()->check(this->checked_instance, ctx));
		} else {
			this->set_result(true); //always true if there are no conditions
		}
	}

private:
	const condition<T> *get_condition() const
	{
		return this->condition;
	}

private:
	const condition<T> *condition = nullptr;
	const T *checked_instance = nullptr;
};

}
