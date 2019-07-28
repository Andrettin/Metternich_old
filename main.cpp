#include "defines.h"
#include "landed_title.h"
#include "map/map.h"
#include "map/province.h"
#include "map/province_image_provider.h"
#include "metternich.h"

#include "third_party/maskedmousearea/maskedmousearea.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

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
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	try {
		LoadDatabase();
		Map::Load();
	} catch (const std::exception &exception) {
		std::cerr << exception.what() << '\n';
	}

	QQmlApplicationEngine engine;

	qmlRegisterType<MaskedMouseArea>("MaskedMouseArea", 1, 0, "MaskedMouseArea");

	qmlRegisterType<Province>();
	engine.rootContext()->setContextProperty("Metternich", Metternich::GetInstance());
	engine.addImageProvider(QLatin1String("provinces"), new ProvinceImageProvider);

	const QUrl url(QStringLiteral("./main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl) {
			QCoreApplication::exit(-1);
		}
	}, Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}
