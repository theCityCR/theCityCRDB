#pragma once

#include "atlasdb/storage/row.hpp"

#include <string>
#include <vector>

namespace atlasdb {

struct QueryResult {
    bool success{true};
    std::string message;
    std::vector<std::string> columns;
    std::vector<Row> rows;
};

}  // namespace atlasdb
