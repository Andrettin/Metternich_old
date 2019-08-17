#include "map/map.h"

#include "map/province.h"
#include "map/terrain.h"
#include "util.h"

#include <QImage>
#include <QRect>

namespace Metternich {

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
	QImage terrain_image("./map/terrain.png"); //used to calculate each province's terrain
	Map::Size = province_image.size(); //set the map's size to that of the province map
	const int pixel_count = province_image.width() * province_image.height();

	std::map<Province *, std::vector<int>> province_pixel_indexes;
	std::map<Province *, std::map<Terrain *, int>> province_terrain_counts;

	const QRgb *rgb_data = reinterpret_cast<const QRgb *>(province_image.constBits());
	const QRgb *terrain_rgb_data = reinterpret_cast<const QRgb *>(terrain_image.constBits());
	for (int i = 0; i < pixel_count; ++i) {
		const QRgb &pixel_rgb = rgb_data[i];

		Province *pixel_province = Province::GetByRGB(pixel_rgb, false);
		if (pixel_province != nullptr) {
			province_pixel_indexes[pixel_province].push_back(i);

			const QRgb &terrain_pixel_rgb = terrain_rgb_data[i];
			Terrain *pixel_terrain = Terrain::GetByRGB(terrain_pixel_rgb);
			std::map<Terrain *, int> &province_terrain_count = province_terrain_counts[pixel_province];
			if (province_terrain_count.find(pixel_terrain) == province_terrain_count.end()) {
				province_terrain_count[pixel_terrain] = 0;
			}
			province_terrain_count[pixel_terrain]++;
		}
	}

	for (const auto &province_terrain_count : province_terrain_counts) {
		Province *province = province_terrain_count.first;
		Terrain *best_terrain = nullptr;
		int best_terrain_count = 0;
		for (const auto &kv_pair : province_terrain_count.second) {
			Terrain *terrain = kv_pair.first;
			const int count = kv_pair.second;
			if (count > best_terrain_count) {
				best_terrain = terrain;
				best_terrain_count = count;
			}
		}
		province->SetTerrain(best_terrain);
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

}
