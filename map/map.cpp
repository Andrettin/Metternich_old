#include "map/map.h"

#include "map/province.h"
#include "util.h"

#include <QImage>
#include <QRect>

#include <set>

void Map::Load()
{
	Map::LoadProvinces();
	Map::LoadTerrain();
}

QPoint Map::GetPixelPosition(const int index)
{
	return IndexToPoint(index, Map::Size);
}

void Map::LoadProvinces()
{
	QImage province_image("./map/provinces.png");
	Map::Size = province_image.size(); //set the map's size to that of the province map
	const int pixel_count = province_image.width() * province_image.height();

	std::map<Province *, std::set<int>> province_pixel_indexes;

	const QRgb *rgb_data = reinterpret_cast<const QRgb *>(province_image.constBits());
	for (int i = 0; i < pixel_count; ++i) {
		const QRgb &pixel_rgb = rgb_data[i];

		Province *pixel_province = Province::GetByRGB(pixel_rgb);
		if (pixel_province != nullptr) {
			province_pixel_indexes[pixel_province].insert(i);
		}
	}

	for (const auto &kv_pair : province_pixel_indexes) {
		Province *province = kv_pair.first;
		province->CreateImage(kv_pair.second);
	}
}

void Map::LoadTerrain()
{
	QImage terrain_image("./map/terrain.png");
}
