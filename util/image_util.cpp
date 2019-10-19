#include "util/image_util.h"

#include "util/container_util.h"

namespace metternich::util {

/**
**	@brief	Get the RGBs present in an image
**
**	@param	image	The image
**
**	@return	The set of RGB values
*/
std::set<QRgb> get_image_rgbs(const QImage &image)
{
	if (!image.colorTable().empty()) {
		return util::container_to_set(image.colorTable());
	}

	std::set<QRgb> rgb_set;

	const int pixel_count = image.width() * image.height();
	const QRgb *rgb_data = reinterpret_cast<const QRgb *>(image.constBits());

	for (int i = 0; i < pixel_count; ++i) {
		rgb_set.insert(rgb_data[i]);
	}

	return rgb_set;
}

}
