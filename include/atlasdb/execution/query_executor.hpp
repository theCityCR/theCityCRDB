#pragma once

#include "atlasdb/execution/query_result.hpp"
#include "atlasdb/parser/ast.hpp"
#include "atlasdb/storage/database.hpp"

#include <memory>
#include <string>

namespace atlasdb {

class QueryExecutor {
public:
    QueryExecutor() = default;

    [[nodiscard]] QueryResult execute(const Query& query);
    [[nodiscard]] std::shared_ptr<Database> currentDatabase() const noexcept;

private:
    [[nodiscard]] QueryResult executeCreateDatabase(const CreateDatabase& command);
    [[nodiscard]] QueryResult executeCreateTable(const CreateTable& command);
    [[nodiscard]] QueryResult executeInsert(const Insert& command);
    [[nodiscard]] QueryResult executeSelect(const Select& command);
    [[nodiscard]] QueryResult executeUpdate(const Update& command);
    [[nodiscard]] QueryResult executeDelete(const Delete& command);

    [[nodiscard]] bool matches(const Row& row, const Table& table, const Predicate& predicate) const;
    [[nodiscard]] std::shared_ptr<Table> requireTable(std::string_view tableName) const;

    std::shared_ptr<Database> database_;
};

}  // namespace atlasdb
