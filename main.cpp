#include "defines.h"
#include "landed_title.h"
#include "metternich.h"
#include "province.h"

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
	} catch (const std::exception &exception) {
		std::cerr << exception.what() << '\n';
	}

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("Metternich", Metternich::GetInstance());
	const QUrl url(QStringLiteral("./main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl) {
			QCoreApplication::exit(-1);
		}
	}, Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}
