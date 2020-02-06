#pragma once

namespace metternich {

class character;
class condition_check_base;
class gsml_data;
class gsml_property;
class holding;
class holding_slot;
class population_unit;
class province;
enum class gsml_operator;
struct context;

template <typename T>
class condition
{
public:
	static std::unique_ptr<condition> from_gsml_property(const gsml_property &property);
	static std::unique_ptr<condition> from_gsml_scope(const gsml_data &scope);

	condition(const gsml_operator condition_operator) : condition_operator(condition_operator)
	{
	}

	virtual ~condition()
	{
	}

	virtual void process_gsml_property(const gsml_property &property);
	virtual void process_gsml_scope(const gsml_data &scope);

	virtual const std::string &get_identifier() const = 0;

	bool check(const T *scope) const;

	virtual bool check_assignment(const T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The assignment operator is not supported for \"" + this->get_identifier() + "\" conditions.");
	}

	virtual bool check_equality(const T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The equality operator is not supported for \"" + this->get_identifier() + "\" conditions.");
	}

	virtual bool check_inequality(const T *scope) const
	{
		return !this->check_equality(scope);
	}

	virtual bool check_less_than(const T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The less than operator is not supported for \"" + this->get_identifier() + "\" conditions.");
	}

	virtual bool check_less_than_or_equality(const T *scope) const
	{
		return this->check_equality(scope) || this->check_less_than(scope);
	}

	virtual bool check_greater_than(const T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The greater than operator is not supported for \"" + this->get_identifier() + "\" conditions.");
	}

	virtual bool check_greater_than_or_equality(const T *scope) const
	{
		return this->check_equality(scope) || this->check_greater_than(scope);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const
	{
		Q_UNUSED(check)
		Q_UNUSED(scope)
	}

	gsml_operator get_operator() const
	{
		return this->condition_operator;
	}

	std::string get_string(const T *scope, const context &ctx, const size_t indent) const;

	virtual std::string get_assignment_string() const
	{
		throw std::runtime_error("The assignment operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_assignment_string(const T *scope, const context &ctx, const size_t indent) const
	{
		Q_UNUSED(scope)
		Q_UNUSED(ctx)
		Q_UNUSED(indent)
		return this->get_assignment_string();
	}

	virtual std::string get_equality_string() const
	{
		throw std::runtime_error("The equality operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_inequality_string() const
	{
		throw std::runtime_error("The inequality operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_less_than_string() const
	{
		throw std::runtime_error("The less than operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_less_than_or_equality_string() const
	{
		throw std::runtime_error("The less than or equality operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_greater_than_string() const
	{
		throw std::runtime_error("The greater than operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_greater_than_or_equality_string() const
	{
		throw std::runtime_error("The greater than or equality operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual bool is_hidden() const
	{
		return false;
	}

private:
	gsml_operator condition_operator;
};

extern template class condition<character>;
extern template class condition<holding>;
extern template class condition<holding_slot>;
extern template class condition<population_unit>;
extern template class condition<province>;

}
