#include "map/map.h"

#include "engine_interface.h"
#include "map/province.h"
#include "map/terrain.h"
#include "util.h"

#include <QImage>
#include <QRect>

namespace Metternich {

void Map::Load()
{
	this->LoadProvinces();
	this->LoadTerrain();
}

QPoint Map::GetPixelPosition(const int index)
{
	return IndexToPoint(index, this->Size);
}

void Map::LoadProvinces()
{
	EngineInterface::Get()->SetLoadingMessage("Loading Provinces... (0%)");

	QImage province_image("./map/provinces.png");
	QImage terrain_image("./map/terrain.png"); //used to calculate each province's terrain
	this->Size = province_image.size(); //set the map's size to that of the province map
	const int pixel_count = province_image.width() * province_image.height();

	std::map<Province *, std::vector<int>> province_pixel_indexes;
	std::map<Province *, std::vector<int>> province_border_pixel_indexes;
	std::map<Province *, std::map<Terrain *, int>> province_terrain_counts;

	const QRgb *rgb_data = reinterpret_cast<const QRgb *>(province_image.constBits());
	const QRgb *terrain_rgb_data = reinterpret_cast<const QRgb *>(terrain_image.constBits());

	Province *previous_pixel_province = nullptr; //used to see which provinces border which horizontally
	for (int i = 0; i < pixel_count; ++i) {
		if ((i % province_image.width()) == 0) {
			//new line, set the previous pixel province to null
			previous_pixel_province = nullptr;

			//update the progress in the loading message
			const long long int progress_percent = static_cast<long long int>(i) * 100 / pixel_count;
			EngineInterface::Get()->SetLoadingMessage("Loading Provinces... (" + QString::number(progress_percent) + "%)");
		}

		const QRgb &pixel_rgb = rgb_data[i];

		Province *pixel_province = Province::GetByRGB(pixel_rgb, false);
		if (pixel_province != nullptr) {
			province_pixel_indexes[pixel_province].push_back(i);

			if (previous_pixel_province != pixel_province && previous_pixel_province != nullptr) {
				province_border_pixel_indexes[pixel_province].push_back(i);
				province_border_pixel_indexes[previous_pixel_province].push_back(i - 1);
				pixel_province->AddBorderProvince(previous_pixel_province);
				previous_pixel_province->AddBorderProvince(pixel_province);
			}

			if (i > province_image.width()) { //second line or below
				//the pixel just above this one
				const QRgb &previous_vertical_pixel_rgb = rgb_data[i - province_image.width()];
				Province *previous_vertical_pixel_province = Province::GetByRGB(previous_vertical_pixel_rgb, false);
				if (previous_vertical_pixel_province != pixel_province && previous_vertical_pixel_province != nullptr) {
					province_border_pixel_indexes[pixel_province].push_back(i);
					province_border_pixel_indexes[previous_vertical_pixel_province].push_back(i - province_image.width());
					pixel_province->AddBorderProvince(previous_vertical_pixel_province);
					previous_vertical_pixel_province->AddBorderProvince(pixel_province);
				}
			}

			const QRgb &terrain_pixel_rgb = terrain_rgb_data[i];
			Terrain *pixel_terrain = Terrain::GetByRGB(terrain_pixel_rgb);
			std::map<Terrain *, int> &province_terrain_count = province_terrain_counts[pixel_province];
			if (province_terrain_count.find(pixel_terrain) == province_terrain_count.end()) {
				province_terrain_count[pixel_terrain] = 0;
			}
			province_terrain_count[pixel_terrain]++;
		}

		previous_pixel_province = pixel_province;
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

	for (const auto &kv_pair : province_border_pixel_indexes) {
		Province *province = kv_pair.first;
		province->SetBorderPixels(kv_pair.second);
	}
}

void Map::LoadTerrain()
{
	EngineInterface::Get()->SetLoadingMessage("Loading Terrain...");
	QImage terrain_image("./map/terrain.png");
}

}
