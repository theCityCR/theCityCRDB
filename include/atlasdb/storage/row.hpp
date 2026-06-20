#pragma once

#include "atlasdb/common/value.hpp"

#include <vector>

namespace atlasdb {

using Row = std::vector<Value>;
using RowId = std::size_t;

}  // namespace atlasdb
