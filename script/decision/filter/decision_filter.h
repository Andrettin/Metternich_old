#pragma once

#include "database/identifiable_type.h"

namespace metternich {

class character;
class holding;
struct context;

template <typename T>
class scoped_decision;

class decision_filter_base
{
public:
	decision_filter_base(const std::string &identifier) : identifier(identifier)
	{
	}

	const std::string &get_identifier() const
	{
		return this->identifier;
	}

private:
	std::string identifier;
};

template <typename T>
class decision_filter : public decision_filter_base, public identifiable_type<decision_filter<T>>
{
public:
	static constexpr const char *class_identifier = "event_trigger";

	decision_filter(const std::string &identifier) : decision_filter_base(identifier)
	{
	}

	void add_decision(const scoped_decision<T> *decision)
	{
		this->decisions.push_back(decision);
	}

	void do_ai_decisions(const std::vector<T *> &scopes, character *source_character) const;

private:
	std::vector<const scoped_decision<T> *> decisions;
};

extern template class decision_filter<holding>;

class holding_decision_filter : public decision_filter<holding>
{
public:
	static inline const decision_filter<holding> *owned = decision_filter::add("owned");
};

}
