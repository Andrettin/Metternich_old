#pragma once

#include <QPoint>
#include <QSize>

class Province;

class Map
{
public:
	static void Load();

	static size_t GetPixelIndex(const int x, const int y)
	{
		return static_cast<size_t>(x + y * Map::Size.width());
	}

	static QPoint GetPixelPosition(const int index)
	{
		return QPoint(index % Map::Size.width(), index / Map::Size.width());
	}

private:
	static void LoadProvinces();
	static void LoadTerrain();

private:
	static inline QSize Size = QSize(0, 0);
};
