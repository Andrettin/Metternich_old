#include "landed_title.h"
#include "province.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <iostream>
#include <stdexcept>

static void LoadDatabase()
{
	LandedTitle::LoadDatabase();
	Province::LoadDatabase();
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	try {
		LoadDatabase();
	} catch (const std::exception &exception) {
		std::cerr << exception.what() << '\n';
	}

	QQmlApplicationEngine engine;
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl) {
			QCoreApplication::exit(-1);
		}
	}, Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}
