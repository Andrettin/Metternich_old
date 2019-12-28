#include "technology/technology.h"

#include "util/container_util.h"

namespace metternich {

const std::filesystem::path &technology::get_icon_path() const
{
	const std::string &base_tag = this->get_icon_tag();

	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

QVariantList technology::get_required_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->get_required_technologies());
}

}
