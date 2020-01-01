#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class character;
enum class government_type_group;

template <typename T>
class condition;

class government_type : public data_entry, public data_type<government_type>
{
	Q_OBJECT

	Q_PROPERTY(metternich::government_type_group group MEMBER group READ get_group)

public:
	static constexpr const char *class_identifier = "government_type";
	static constexpr const char *database_folder = "government_types";

	government_type(const std::string &identifier);
	virtual ~government_type() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	government_type_group get_group() const
	{
		return this->group;
	}

	const condition<character> *get_conditions() const
	{
		return this->conditions.get();
	}

private:
	government_type_group group;
	std::unique_ptr<condition<character>> conditions;
};

}
