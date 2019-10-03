#include "province_image_provider.h"

#include "province.h"

namespace metternich {

QImage province_image_provider::requestImage(const QString &id, QSize *size, const QSize &)
{
	std::string identifier = id.toStdString();
	const province *province = province::get(identifier);

	if (size != nullptr) {
		*size = province->get_image().size();
	}

	return province->get_image();
}

}
