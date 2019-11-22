#pragma once

#include <QObject>
#include <QVariant>

#include <vector>

namespace metternich::util {

template <typename T>
inline QVariantList container_to_qvariant_list(const T &container)
{
	QVariantList list;

	for (const typename T::value_type &element : container) {
		list.append(QVariant::fromValue(element));
	}

	return list;
}

template <typename T>
inline QObjectList vector_to_qobject_list(const std::vector<T *> &vector)
{
	QObjectList object_list;
	for (T *element : vector) {
		object_list.append(element);
	}
	return object_list;
}

template <typename T>
inline std::set<typename T::value_type> container_to_set(const T &container)
{
	std::set<typename T::value_type> set;

	for (const typename T::value_type &element : container) {
		set.insert(element);
	}

	return set;
}

template <typename T>
inline std::vector<typename T::value_type> container_to_vector(const T &container)
{
	std::vector<typename T::value_type> vector;

	for (const typename T::value_type &element : container) {
		vector.push_back(element);
	}

	return vector;
}

}
