#include "atlasdb/execution/query_executor.hpp"
#include "atlasdb/parser/parser.hpp"

#include <iostream>
#include <sstream>

namespace {

void printResult(const atlasdb::QueryResult& result) {
    if (!result.message.empty()) {
        std::cout << result.message << '\n';
    }
    if (result.rows.empty()) {
        return;
    }

    for (const auto& column : result.columns) {
        std::cout << column << '\t';
    }
    std::cout << '\n';

    for (const auto& row : result.rows) {
        for (const auto& value : row) {
            std::cout << value << '\t';
        }
        std::cout << '\n';
    }
}

}  // namespace

int main() {
    atlasdb::Parser parser;
    atlasdb::QueryExecutor executor;

    std::cout << "AtlasDB interactive shell. Type EXIT; to quit.\n";
    for (std::string line; std::cout << "atlasdb> " && std::getline(std::cin, line);) {
        if (line.empty()) {
            continue;
        }
        try {
            auto query = parser.parse(line);
            auto result = executor.execute(query);
            printResult(result);
            if (result.message == "exit") {
                break;
            }
        } catch (const std::exception& ex) {
            std::cerr << "error: " << ex.what() << '\n';
        }
    }

    return 0;
}
