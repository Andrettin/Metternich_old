#pragma once

#include <QPoint>
#include <QSize>

namespace Metternich {

class Province;

class Map
{
public:
	static void Load();
	static QPoint GetPixelPosition(const int index);

private:
	static void LoadProvinces();
	static void LoadTerrain();

private:
	static inline QSize Size = QSize(0, 0);
};

}
