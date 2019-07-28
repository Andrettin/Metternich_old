#include "map.h"

#include "province.h"

#include <QImage>

#include <set>

void Map::Load()
{
	Map::LoadProvinces();
	Map::LoadTerrain();
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

		Province *pixel_province = Province::Get(pixel_rgb);
		if (pixel_province != nullptr) {
			province_pixel_indexes[pixel_province].insert(i);
		}
	}

	for (const auto &kv_pair : province_pixel_indexes) {
		Province *province = kv_pair.first;
		const std::set<int> &pixel_indexes = kv_pair.second;
		QPoint start_pos(-1, -1);
		QPoint end_pos(-1, -1);

		for (const int index : pixel_indexes) {
			QPoint pixel_pos = Map::GetPixelPosition(index);
			if (start_pos.x() == -1 || pixel_pos.x() < start_pos.x()) {
				start_pos.setX(pixel_pos.x());
			}
			if (start_pos.y() == -1 || pixel_pos.y() < start_pos.y()) {
				start_pos.setY(pixel_pos.y());
			}
			if (end_pos.x() == -1 || pixel_pos.x() > end_pos.x()) {
				end_pos.setX(pixel_pos.x());
			}
			if (end_pos.y() == -1 || pixel_pos.y() > end_pos.y()) {
				end_pos.setY(pixel_pos.y());
			}
		}

		QRect rect(start_pos, end_pos);
		province->SetRect(rect);
	}
}

void Map::LoadTerrain()
{
	QImage terrain_image("./map/terrain.png");
}
