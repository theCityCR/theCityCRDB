#pragma once

#include <cstdint>

namespace theCityCRDB {

enum class ComparisonOperator : std::uint8_t {
    Equal,
    Greater,
    Less,
};

} // namespace theCityCRDB
