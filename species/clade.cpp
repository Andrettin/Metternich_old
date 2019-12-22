#include "species/clade.h"

#include "game/game.h"
#include "map/province.h"
#include "species/species.h"
#include "util/container_util.h"

namespace metternich {

QVariantList clade::get_species_qvariant_list() const
{
	return container::to_qvariant_list(this->get_species());
}

Q_INVOKABLE void clade::add_species(metternich::species *species)
{
	this->species.insert(species);

	if (species->get_clade() != this) {
		species->set_clade(this);
	}
}

Q_INVOKABLE void clade::remove_species(metternich::species *species)
{
	this->species.erase(species);

	if (species->get_clade() == this) {
		species->set_clade(nullptr);
	}
}

QVariantList clade::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

bool clade::is_ai() const
{
	return game::get()->get_player_clade() != this;
}

}
