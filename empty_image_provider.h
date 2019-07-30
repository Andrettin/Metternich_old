#pragma once

#include <QQuickImageProvider>

namespace Metternich {

class EmptyImageProvider : public QQuickImageProvider
{
public:
	EmptyImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

	virtual QImage requestImage(const QString &, QSize *, const QSize &) override
	{
		QImage image(QSize(1, 1), QImage::Format_ARGB32);
		image.fill(qRgba(0, 0, 0, 0));
		return image;
	}
};

}
