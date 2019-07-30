#pragma once

#include <QQuickImageProvider>

namespace Metternich {

class ProvinceImageProvider : public QQuickImageProvider
{
public:
	ProvinceImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

	virtual QImage requestImage(const QString &id, QSize *size, const QSize &requested_size) override;
};

}
