#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/database.h"
#include "defines.h"
#include "economy/commodity.h"
#include "empty_image_provider.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/province.h"
#include "map/province_image_provider.h"
#include "map/region.h"
#include "map/terrain_type.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "religion.h"
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
			Game::get()->Start(Defines::get()->GetStartDate());
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

		Translator *translator = Translator::get();

		translator->load_locale("english");

		app.installTranslator(translator);

		std::thread data_load_thread(load_data);
		data_load_thread.detach();

		QQmlApplicationEngine engine;

		qmlRegisterType<MaskedMouseArea>("MaskedMouseArea", 1, 0, "MaskedMouseArea");

		qmlRegisterType<building>();
		qmlRegisterType<commodity>();
		qmlRegisterType<culture>();
		qmlRegisterType<Game>();
		qmlRegisterType<holding>();
		qmlRegisterType<holding_type>();
		qmlRegisterType<landed_title>();
		qmlRegisterType<population_type>();
		qmlRegisterType<population_unit>();
		qmlRegisterType<province>();
		qmlRegisterType<religion>();
		qmlRegisterType<terrain_type>();
		engine.rootContext()->setContextProperty("metternich", EngineInterface::get());
		engine.addImageProvider(QLatin1String("provinces"), new province_image_provider);
		engine.addImageProvider(QLatin1String("empty"), new EmptyImageProvider);

		const QUrl url(QStringLiteral("./interface/main.qml"));
		QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
			if (!obj && url == objUrl) {
				QCoreApplication::exit(-1);
			}
		}, Qt::QueuedConnection);
		engine.load(url);

		return app.exec();
	} catch (const std::exception &exception) {
		qCritical() << exception.what() << '\n';
		return -1;
	}
}
