#pragma once

#include <memory>
#include <string>

namespace metternich {

class character;
class gsml_data;
class gsml_property;
class holding;
class population_unit;
class province;
enum class gsml_operator : int;

static constexpr const char *no_effect_string = "No effect";

template <typename T>
class effect
{
public:
	static std::unique_ptr<effect> from_gsml_property(const gsml_property &property);
	static std::unique_ptr<effect> from_gsml_scope(const gsml_data &scope);

	effect(const gsml_operator effect_operator);
	virtual ~effect() {}

	virtual const std::string &get_identifier() const = 0;

	virtual void process_gsml_property(const gsml_property &property);
	virtual void process_gsml_scope(const gsml_data &scope);

	void do_effect(T *scope) const;

	virtual void do_assignment_effect(T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The assignment operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual void do_addition_effect(T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The addition operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual void do_subtraction_effect(T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("The subtraction operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	gsml_operator get_operator() const
	{
		return this->effect_operator;
	}

	std::string get_string(const T *scope, const size_t indent) const;

	virtual std::string get_assignment_string() const
	{
		throw std::runtime_error("The assignment operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_assignment_string(const T *scope, const size_t indent) const
	{
		Q_UNUSED(scope)
		Q_UNUSED(indent)
		return this->get_assignment_string();
	}

	virtual std::string get_addition_string() const
	{
		throw std::runtime_error("The addition operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual std::string get_subtraction_string() const
	{
		throw std::runtime_error("The subtraction operator is not supported for \"" + this->get_identifier() + "\" effects.");
	}

	virtual bool is_hidden() const
	{
		return false;
	}

private:
	gsml_operator effect_operator;
};

extern template class effect<character>;
extern template class effect<holding>;
extern template class effect<province>;

}
