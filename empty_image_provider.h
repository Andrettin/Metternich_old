#pragma once

#include <QQuickImageProvider>

namespace metternich {

class empty_image_provider : public QQuickImageProvider
{
public:
	empty_image_provider() : QQuickImageProvider(QQuickImageProvider::Image) {}

	virtual QImage requestImage(const QString &, QSize *, const QSize &) override
	{
		QImage image(QSize(1, 1), QImage::Format_ARGB32);
		image.fill(qRgba(0, 0, 0, 0));
		return image;
	}
};

}
