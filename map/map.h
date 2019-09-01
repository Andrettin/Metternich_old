#pragma once

#include "singleton.h"

#include <QPoint>
#include <QSize>

namespace Metternich {

class Province;

class Map : public Singleton<Map>
{
public:
	void Load();
	QPoint GetPixelPosition(const int index);

private:
	void LoadProvinces();
	void LoadTerrain();

private:
	QSize Size = QSize(0, 0);
};

}
