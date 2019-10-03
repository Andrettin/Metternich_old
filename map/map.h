#pragma once

#include "singleton.h"

#include <QPoint>
#include <QSize>

namespace metternich {

class map : public singleton<map>
{
public:
	void load();
	QPoint get_pixel_position(const int index);

private:
	void load_provinces();
	void load_terrain();

private:
	QSize size = QSize(0, 0);
};

}
