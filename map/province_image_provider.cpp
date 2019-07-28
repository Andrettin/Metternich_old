#include "province_image_provider.h"

#include "province.h"

QImage ProvinceImageProvider::requestImage(const QString &id, QSize *size, const QSize &)
{
	const Province *province = Province::Get(id.toStdString());

	if (size != nullptr) {
		*size = province->GetImage().size();
	}

	return province->GetImage();
}
