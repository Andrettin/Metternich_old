#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <map>
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

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();

		if (tag == "mixing_results") {
			scope.for_each_property([&](const gsml_property &property) {
				const std::string &key = property.get_key();
				const std::string &value = property.get_value();

				phenotype *mixing_phenotype = phenotype::get(key);
				phenotype *result_phenotype = phenotype::get(value);
				this->mixing_results[mixing_phenotype] = result_phenotype;
				mixing_phenotype->mixing_results[this] = result_phenotype;
			});
		} else {
			data_entry_base::process_gsml_scope(scope);
		}
	}

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

	bool can_mix_with(const phenotype *other_phenotype) const
	{
		if (this == other_phenotype) {
			return false;
		}

		return this->mixing_results.contains(other_phenotype);
	}

	phenotype *get_mixing_result(const phenotype *other_phenotype) const
	{
		auto find_iterator = this->mixing_results.find(other_phenotype);
		if (find_iterator != this->mixing_results.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

private:
	phenotype *icon_fallback_phenotype = nullptr; //the phenotype from which the tag is to be used if an icon for this phenotype is absent
	std::map<const phenotype *, phenotype *> mixing_results; //the result of mixing with another phenotype
};

}
