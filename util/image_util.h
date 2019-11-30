#pragma once

#include <QColor>
#include <QImage>

namespace metternich::image {

extern std::set<QRgb> get_rgbs(const QImage &image);

}
