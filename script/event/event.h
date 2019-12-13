#pragma once

#include "database/data_entry.h"

namespace metternich {

class event : public data_entry
{
	Q_OBJECT

public:
	event(const std::string &identifier) : data_entry(identifier) {}
};

}
