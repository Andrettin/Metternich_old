#include "map/world_type.h"

#include "database/database.h"

namespace metternich {

const std::filesystem::path &world_type::get_texture_path() const
{
	//returns a random texture for the world type
	return database::get()->get_tagged_texture_path(this->get_texture_tag());
}

}
