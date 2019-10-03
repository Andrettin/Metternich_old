#pragma once

#include "singleton.h"

#include <QPoint>
#include <QSize>

namespace metternich {

class Map : public singleton<Map>
{
public:
	void load();
	QPoint GetPixelPosition(const int index);

private:
	void load_provinces();
	void load_terrain();

private:
	QSize Size = QSize(0, 0);
};

}
