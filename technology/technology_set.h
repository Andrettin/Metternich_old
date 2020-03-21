#pragma once

#include "technology/technology_compare.h"

#include <set>

namespace metternich {

using technology_set = std::set<technology *, technology_compare>;

}
