#include "defines.h"
#include "empty_image_provider.h"
#include "game.h"
#include "landed_title.h"
#include "map/map.h"
#include "map/province.h"
#include "map/province_image_provider.h"
#include "metternich.h"
#include "translator.h"

#include "third_party/maskedmousearea/maskedmousearea.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include <iostream>
#include <stdexcept>

static void LoadDatabase()
{
	Defines::Load();
	LandedTitle::LoadDatabase();
	Province::LoadDatabase();
}

int main(int argc, char *argv[])
{
	try {
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

		QGuiApplication app(argc, argv);

		Translator translator;

		LoadDatabase();
		Map::Load();
		translator.LoadLocale("english");

		app.installTranslator(&translator);

		Game::GetInstance()->Start(Defines::GetStartDate());

		QQmlApplicationEngine engine;

		qmlRegisterType<MaskedMouseArea>("MaskedMouseArea", 1, 0, "MaskedMouseArea");

		qmlRegisterType<Game>();
		qmlRegisterType<LandedTitle>();
		qmlRegisterType<Province>();
		engine.rootContext()->setContextProperty("Metternich", Metternich::GetInstance());
		engine.addImageProvider(QLatin1String("provinces"), new ProvinceImageProvider);
		engine.addImageProvider(QLatin1String("empty"), new EmptyImageProvider);

		const QUrl url(QStringLiteral("./main.qml"));
		QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
			if (!obj && url == objUrl) {
				QCoreApplication::exit(-1);
			}
		}, Qt::QueuedConnection);
		engine.load(url);

		return app.exec();
	} catch (const std::exception &exception) {
		std::cerr << exception.what() << '\n';
		return -1;
	}
}
