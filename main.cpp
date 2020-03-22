#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "database/database.h"
#include "database/defines.h"
#include "economy/commodity.h"
#include "economy/trade_node.h"
#include "game/engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/province.h"
#include "map/region.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "politics/government_type.h"
#include "politics/law.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "religion/religion.h"
#include "technology/technology.h"
#include "util/empty_image_provider.h"
#include "util/exception_util.h"
#include "util/translator.h"

#include "third_party/maskedmousearea/maskedmousearea.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

namespace metternich {
	static void load_data()
	{
		try {
			database::get()->load();
			map::get()->load();
			database::get()->initialize();
			map::get()->calculate_cosmic_map_bounding_rect();
			game::get()->start(defines::get()->get_default_timeline(), defines::get()->get_start_date());
		} catch (const std::exception &exception) {
			exception::report(exception);
			QMetaObject::invokeMethod(QApplication::instance(), []{ QApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
		}
	}
}

int main(int argc, char *argv[])
{
	using namespace metternich;

	try {
		QApplication app(argc, argv);

		translator *translator = translator::get();
		translator->set_locale("english");

		database::get()->process_modules();

		translator->load();
		app.installTranslator(translator);

		std::thread data_load_thread(load_data);

		QQmlApplicationEngine engine;

		qmlRegisterType<MaskedMouseArea>("MaskedMouseArea", 1, 0, "MaskedMouseArea");

		qmlRegisterType<building>();
		qmlRegisterType<commodity>();
		qmlRegisterType<culture>();
		qmlRegisterType<game>();
		qmlRegisterType<government_type>();
		qmlRegisterType<holding>();
		qmlRegisterType<holding_slot>();
		qmlRegisterType<holding_type>();
		qmlRegisterType<landed_title>();
		qmlRegisterType<law>();
		qmlRegisterType<population_type>();
		qmlRegisterType<population_unit>();
		qmlRegisterType<province>();
		qmlRegisterType<religion>();
		qmlRegisterType<technology>();
		qmlRegisterType<terrain_type>();
		qmlRegisterType<territory>();
		qmlRegisterType<trade_node>();
		qmlRegisterType<world>();
		engine.rootContext()->setContextProperty("metternich", engine_interface::get());
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
		exception::report(exception);
		return -1;
	}
}
