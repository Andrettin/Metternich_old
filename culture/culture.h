#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

#include <memory>
#include <string>
#include <vector>

namespace metternich {

class culture_group;
class culture_supergroup;
class dynasty;
class population_unit;

template <typename T>
class condition;

class culture final : public culture_base, public data_type<culture>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture_group* group MEMBER group READ get_group NOTIFY group_changed)
	Q_PROPERTY(QVariantList derived_cultures READ get_derived_cultures_qvariant_list)

public:
	static constexpr const char *class_identifier = "culture";
	static constexpr const char *database_folder = "cultures";

	culture(const std::string &identifier);
	~culture() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	culture_group *get_group() const
	{
		return this->group;
	}

	culture_supergroup *get_supergroup() const;

	const std::set<culture *> &get_derived_cultures() const
	{
		return this->derived_cultures;
	}

	QVariantList get_derived_cultures_qvariant_list() const;

	Q_INVOKABLE void add_derived_culture(culture *culture)
	{
		this->derived_cultures.insert(culture);
	}

	Q_INVOKABLE void remove_derived_culture(culture *culture)
	{
		this->derived_cultures.erase(culture);
	}

	const condition<population_unit> *get_derivation_conditions() const
	{
		return this->derivation_conditions.get();
	}

	std::string generate_male_name() const;
	std::string generate_female_name() const;
	std::string generate_dynasty_name() const;

signals:
	void group_changed();

private:
	culture_group *group = nullptr;
	std::set<culture *> derived_cultures;
	std::unique_ptr<condition<population_unit>> derivation_conditions;
};

}
