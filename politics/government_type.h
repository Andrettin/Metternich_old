#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class character;
class holding_type;
enum class government_type_group;

template <typename T>
class condition;

class government_type : public data_entry, public data_type<government_type>
{
	Q_OBJECT

	Q_PROPERTY(metternich::government_type_group group MEMBER group READ get_group)
	Q_PROPERTY(QVariantList allowed_holding_types READ get_allowed_holding_types_qvariant_list)

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

	const std::set<holding_type *> &get_allowed_holding_types() const
	{
		return this->allowed_holding_types;
	}

	QVariantList get_allowed_holding_types_qvariant_list() const;

	Q_INVOKABLE void add_allowed_holding_type(holding_type *holding_type)
	{
		this->allowed_holding_types.insert(holding_type);
	}

	Q_INVOKABLE void remove_allowed_holding_type(holding_type *holding_type)
	{
		this->allowed_holding_types.erase(holding_type);
	}

	const condition<character> *get_conditions() const
	{
		return this->conditions.get();
	}

private:
	government_type_group group;
	std::set<holding_type *> allowed_holding_types;
	std::unique_ptr<condition<character>> conditions;
};

}
