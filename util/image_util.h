#pragma once

#include <QColor>
#include <QImage>

namespace metternich::util {

extern std::set<QRgb> get_image_rgbs(const QImage &image);

}
