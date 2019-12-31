#pragma once

#include <stdexcept>

namespace metternich::exception {

inline void report(const std::exception &exception)
{
	try {
		std::rethrow_if_nested(exception);
	} catch (const std::exception &nested_exception) {
		exception::report(nested_exception);
	}

	qCritical() << exception.what();
}

}
