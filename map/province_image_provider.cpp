#include "province_image_provider.h"

#include "province.h"

QImage ProvinceImageProvider::requestImage(const QString &id, QSize *size, const QSize &)
{
	std::string identifier = id.toStdString();
	const Province *province = Province::Get(identifier);

	if (size != nullptr) {
		*size = province->GetImage().size();
	}

	return province->GetImage();
}
