#pragma once

#include <QQuickImageProvider>

namespace metternich {

class province_image_provider final : public QQuickImageProvider
{
public:
	province_image_provider() : QQuickImageProvider(QQuickImageProvider::Image) {}

	virtual QImage requestImage(const QString &id, QSize *size, const QSize &requested_size) override;
};

}
