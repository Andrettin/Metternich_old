#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class phenotype : public data_entry, public data_type<phenotype>
{
	Q_OBJECT

	Q_PROPERTY(metternich::phenotype* icon_fallback_phenotype MEMBER icon_fallback_phenotype READ get_icon_fallback_phenotype)

public:
	static constexpr const char *class_identifier = "phenotype";
	static constexpr const char *database_folder = "phenotypes";

	phenotype(const std::string &identifier) : data_entry(identifier) {}

	phenotype *get_icon_fallback_phenotype() const
	{
		return this->icon_fallback_phenotype;
	}

	std::vector<phenotype *> get_icon_fallback_phenotypes() const
	{
		std::vector<phenotype *> fallback_phenotypes;
		if (this->get_icon_fallback_phenotype() != nullptr) {
			fallback_phenotypes.push_back(this->get_icon_fallback_phenotype());

			//get the fallbacks of the parent as well
			for (phenotype *fallback_phenotype : this->get_icon_fallback_phenotype()->get_icon_fallback_phenotypes()) {
				fallback_phenotypes.push_back(fallback_phenotype);
			}
		}
		return fallback_phenotypes;
	}

private:
	phenotype *icon_fallback_phenotype = nullptr; //the phenotype from which the tag is to be used if an icon for this phenotype is absent
};

}
