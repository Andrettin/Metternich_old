#pragma once

#include <QApplication>
#include <QObject>
#include <QThread>

#include <memory>
#include <mutex>
#include <type_traits>

namespace Metternich {

/**
**	@brief	The singleton template class
*/
template <typename T>
class Singleton
{
public:
	/**
	**	@brief	Get the singleton instance
	*/
	static T *Get()
	{
		std::call_once(Singleton<T>::OnceFlag, [](){
			T::Create();
		});

		return Singleton<T>::Instance.get();
	}

private:
	/**
	**	@brief	Create the singleton instance
	*/
	static void Create()
	{
		Singleton<T>::Instance = std::make_unique<T>();

		if constexpr (std::is_base_of_v<QObject, T>) {
			if (QApplication::instance()->thread() != QThread::currentThread()) {
				Singleton<T>::Instance->moveToThread(QApplication::instance()->thread());
			}
		}
	}

private:
	static inline std::unique_ptr<T> Instance;
	static inline std::once_flag OnceFlag;
};

}
