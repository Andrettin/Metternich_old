#include "species/species.h"

#include "species/clade.h"
#include "util/container_util.h"

namespace metternich {

QVariantList species::get_evolutions_qvariant_list() const
{
	return container::to_qvariant_list(this->get_evolutions());
}

void species::set_clade(metternich::clade *clade)
{
	if (clade == this->get_clade()) {
		return;
	}

	if (this->get_clade() != nullptr) {
		this->get_clade()->remove_species(this);
	}

	this->clade = clade;

	if (clade != nullptr) {
		clade->add_species(this);
	}
}

}
