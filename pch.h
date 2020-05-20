#pragma once

#include <type_traits>
namespace std {
	template <typename T>
	using result_of = invoke_result<T>;
}

#include "maskedmousearea/maskedmousearea.h"

#include <QApplication>
#include <QColor>
#include <QCryptographicHash>
#include <QDateTime>
#include <QGeoCircle>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoPolygon>
#include <QGeoRectangle>
#include <QImage>
#include <QJsonDocument>
#include <QList>
#include <QLocale>
#include <QMetaProperty>
#include <QObject>
#include <QPoint>
#include <QPolygonF>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QRect>
#include <QSize>
#include <QStandardPaths>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QTimeZone>
#include <QTranslator>
#include <QUuid>
#include <QVariant>
#include <QVariantList>
#include <QVector>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
