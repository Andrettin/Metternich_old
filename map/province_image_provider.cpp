#include "province_image_provider.h"

#include "province.h"
#include "util/exception_util.h"

namespace metternich {

QImage province_image_provider::requestImage(const QString &id, QSize *size, const QSize &)
{
	try {
		//remove the "?flag=0"/"?flag=1" which is used as a workaround to reload images
		const std::string identifier = id.toStdString().substr(0, static_cast<size_t>(id.size() - 7));
		const province *province = province::get(identifier);

		if (province->get_image().isNull()) {
			QImage image(QSize(1, 1), QImage::Format_ARGB32);
			image.fill(qRgba(0, 0, 0, 0));
			return image; //empty image as a fallback
		}

		if (size != nullptr) {
			*size = province->get_image().size();
		}

		return province->get_image();
	} catch (const std::exception &exception) {
		exception::report(exception);

		QImage image(QSize(1, 1), QImage::Format_ARGB32);
		image.fill(qRgba(0, 0, 0, 0));
		return image; //empty image as a fallback
	}
}

}
