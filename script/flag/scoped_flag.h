#pragma once

#include "database/identifiable_type.h"

#include <string>

namespace metternich {

class scoped_flag_base
{
public:
	scoped_flag_base(const std::string &identifier) : identifier(identifier)
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
class scoped_flag : public scoped_flag_base, public identifiable_type<scoped_flag<T>>
{
public:
	static constexpr const char *class_identifier = "scoped_flag";

	scoped_flag(const std::string &identifier) : scoped_flag_base(identifier)
	{
	}
};

}
