#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "database/database.h"
#include "defines.h"
#include "economy/commodity.h"
#include "empty_image_provider.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/province.h"
#include "map/province_image_provider.h"
#include "map/region.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "religion/religion.h"
#include "translator.h"

#include "third_party/maskedmousearea/maskedmousearea.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include <iostream>
#include <stdexcept>

namespace metternich {
	static void load_data()
	{
		try {
			database::get()->load();
			map::get()->load();
			database::get()->initialize();
			map::get()->set_current_world(defines::get()->get_default_world());
			game::get()->start(defines::get()->get_default_timeline(), defines::get()->get_start_date());
		} catch (const std::exception &exception) {
			qCritical() << exception.what() << '\n';
			QMetaObject::invokeMethod(QApplication::instance(), []{ QApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
		}
	}
}

int main(int argc, char *argv[])
{
	using namespace metternich;

	try {
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

		QApplication app(argc, argv);

		database::get()->process_modules();

		translator *translator = translator::get();
		translator->load_locale("english");
		app.installTranslator(translator);

		std::thread data_load_thread(load_data);

		QQmlApplicationEngine engine;

		qmlRegisterType<MaskedMouseArea>("MaskedMouseArea", 1, 0, "MaskedMouseArea");

		qmlRegisterType<building>();
		qmlRegisterType<commodity>();
		qmlRegisterType<culture>();
		qmlRegisterType<game>();
		qmlRegisterType<holding>();
		qmlRegisterType<holding_slot>();
		qmlRegisterType<holding_type>();
		qmlRegisterType<landed_title>();
		qmlRegisterType<population_type>();
		qmlRegisterType<population_unit>();
		qmlRegisterType<province>();
		qmlRegisterType<religion>();
		qmlRegisterType<terrain_type>();
		qmlRegisterType<world>();
		engine.rootContext()->setContextProperty("metternich", engine_interface::get());
		engine.addImageProvider(QLatin1String("provinces"), new province_image_provider);
		engine.addImageProvider(QLatin1String("empty"), new empty_image_provider);

		const QUrl url(QStringLiteral("./interface/main.qml"));
		QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
			if (!obj && url == objUrl) {
				QCoreApplication::exit(-1);
			}
		}, Qt::QueuedConnection);
		engine.load(url);

		const int result = app.exec();

		if (game::get()->is_running()) {
			game::get()->stop();
		}

		data_load_thread.join();

		return result;
	} catch (const std::exception &exception) {
		qCritical() << exception.what() << '\n';
		return -1;
	}
}
