#pragma once

#include <QObject>

#include <memory>

namespace metternich {

struct qobject_deleter
{
	void operator()(QObject *object)
	{
		object->deleteLater();
	}
};

//unique pointer that calls QObject::deleteLater
template <typename T>
using qunique_ptr = std::unique_ptr<T, qobject_deleter>;

template <typename T, class... ARGS>
inline qunique_ptr<T> make_qunique(ARGS&&... args)
{
	return qunique_ptr<T>(new T(std::forward<ARGS>(args)...));
}

}
