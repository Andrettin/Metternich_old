#pragma once

#include <memory>
#include <mutex>

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
		std::call_once(Singleton<T>::OnceFlag, [](){ Singleton<T>::Instance = std::make_unique<T>(); });

		return Singleton<T>::Instance.get();
	}

private:
	static inline std::unique_ptr<T> Instance;
	static inline std::once_flag OnceFlag;
};

}
