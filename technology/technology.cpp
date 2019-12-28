#include "technology/technology.h"

#include "util/container_util.h"

namespace metternich {

QVariantList technology::get_required_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->get_required_technologies());
}

}
