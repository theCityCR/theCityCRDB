#pragma once

#include "theCityCRDB/execution/query_result.hpp"
#include "theCityCRDB/parser/ast.hpp"
#include "theCityCRDB/persistence/storage_manager.hpp"
#include "theCityCRDB/persistence/write_ahead_log.hpp"
#include "theCityCRDB/planner/query_planner.hpp"
#include "theCityCRDB/storage/database.hpp"
#include "theCityCRDB/transaction/transaction_manager.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace theCityCRDB {

class QueryExecutor {
public:
    explicit QueryExecutor(std::filesystem::path storageRoot = "data");

    [[nodiscard]] QueryResult execute(const Query& query);
    [[nodiscard]] std::shared_ptr<Database> currentDatabase() const noexcept;

private:
    [[nodiscard]] QueryResult executeCreateDatabase(const CreateDatabase& command);
    [[nodiscard]] QueryResult executeCreateTable(const CreateTable& command);
    [[nodiscard]] QueryResult executeDropTable(const DropTable& command);
    [[nodiscard]] QueryResult executeRenameTable(const RenameTable& command);
    [[nodiscard]] QueryResult executeListTables();
    [[nodiscard]] QueryResult executeInsert(const Insert& command);
    [[nodiscard]] QueryResult executeSelect(const Select& command);
    [[nodiscard]] QueryResult executeUpdate(const Update& command);
    [[nodiscard]] QueryResult executeDelete(const Delete& command);
    [[nodiscard]] QueryResult executeCreateIndex(const CreateIndex& command);
    [[nodiscard]] QueryResult executeSaveDatabase();
    [[nodiscard]] QueryResult executeLoadDatabase(const LoadDatabase& command);
    [[nodiscard]] QueryResult executeBegin();
    [[nodiscard]] QueryResult executeCommit();
    [[nodiscard]] QueryResult executeRollback();

    [[nodiscard]] bool matches(const Row& row, const Table& table, const Predicate& predicate) const;
    [[nodiscard]] std::shared_ptr<Table> requireTable(std::string_view tableName) const;

    std::shared_ptr<Database> database_;
    std::shared_ptr<Database> transactionSnapshot_;
    StorageManager storageManager_;
    WriteAheadLog wal_;
    QueryPlanner planner_;
    TransactionManager transactionManager_;
    std::optional<TransactionId> activeTransaction_;
};

}  // namespace theCityCRDB
