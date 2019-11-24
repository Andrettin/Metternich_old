#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

#include <string>
#include <vector>

namespace metternich {

class culture_group;
class dynasty;

class culture : public culture_base, public data_type<culture>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture_group* culture_group MEMBER culture_group READ get_culture_group NOTIFY culture_group_changed)

public:
	static constexpr const char *class_identifier = "culture";
	static constexpr const char *database_folder = "cultures";

	culture(const std::string &identifier) : culture_base(identifier) {}

	virtual void initialize() override;
	virtual void check() const override;

	metternich::culture_group *get_culture_group() const
	{
		return this->culture_group;
	}

	void add_dynasty(dynasty *dynasty)
	{
		this->dynasties.push_back(dynasty);
	}

	void increase_male_name_weight(const std::string &name)
	{
		if (!this->has_male_name(name)) {
			return;
		}

		this->male_name_generation_list.push_back(name);
	}

	std::string generate_male_name() const;

	void increase_female_name_weight(const std::string &name)
	{
		if (!this->has_female_name(name)) {
			return;
		}

		this->female_name_generation_list.push_back(name);
	}

	std::string generate_female_name() const;
	std::string generate_dynasty_name() const;

signals:
	void culture_group_changed();

private:
	metternich::culture_group *culture_group = nullptr;
	std::vector<dynasty *> dynasties;
	std::vector<std::string> male_name_generation_list; //the male name generation list; the content is similar to the male_names set in the culture_base class, except that the same name can appear multiple times in the list, as a form of weighting certain names more than others for generation
	std::vector<std::string> female_name_generation_list;
};

}
