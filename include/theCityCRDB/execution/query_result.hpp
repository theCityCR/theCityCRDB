#pragma once

#include "theCityCRDB/storage/row.hpp"

#include <string>
#include <vector>

namespace theCityCRDB {

struct QueryResult {
    bool success{true};
    std::string message;
    std::vector<std::string> columns;
    std::vector<Row> rows;
};

}  // namespace theCityCRDB
