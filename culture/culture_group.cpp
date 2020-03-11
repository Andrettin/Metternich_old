#include "culture/culture_group.h"

#include "culture/culture_supergroup.h"

namespace metternich {

void culture_group::initialize()
{
	if (this->get_supergroup() == nullptr) {
		throw std::runtime_error("Culture group \"" + this->get_identifier() + "\" has no culture supergroup.");
	}

	if (this->get_species() == nullptr) {
		this->set_species(this->get_supergroup()->get_species());
	}

	if (this->get_default_phenotype() == nullptr) {
		this->set_default_phenotype(this->get_supergroup()->get_default_phenotype());
	}

	if (!this->get_male_names().empty()) {
		this->get_supergroup()->add_male_names(this->get_male_names());
	}

	if (!this->get_female_names().empty()) {
		this->get_supergroup()->add_female_names(this->get_female_names());
	}

	if (!this->get_dynasty_names().empty()) {
		this->get_supergroup()->add_dynasty_names(this->get_dynasty_names());
	}

	data_entry_base::initialize();
}

}
