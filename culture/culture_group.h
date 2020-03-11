#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

namespace metternich {

class culture_supergroup;

class culture_group final : public culture_base, public data_type<culture_group>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture_supergroup* supergroup MEMBER supergroup READ get_supergroup)

public:
	static constexpr const char *class_identifier = "culture_group";
	static constexpr const char *database_folder = "culture_groups";

	culture_group(const std::string &identifier) : culture_base(identifier) {}

	virtual void initialize() override;

	culture_supergroup *get_supergroup() const
	{
		return this->supergroup;
	}

private:
	culture_supergroup *supergroup = nullptr;
};

}
