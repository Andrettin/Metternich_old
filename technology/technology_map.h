#pragma once

#include "technology/technology_compare.h"

#include <map>

namespace metternich {

template <typename T>
using technology_map = std::map<technology *, T, technology_compare>;

}
