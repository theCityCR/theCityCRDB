#include "theCityCRDB/execution/query_executor.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace theCityCRDB {
namespace {

bool compare(const Value &left, ComparisonOperator op, const Value &right) {
    switch (op) {
    case ComparisonOperator::Equal:
        return left == right;
    case ComparisonOperator::Greater:
        return right < left;
    case ComparisonOperator::Less:
        return left < right;
    }
    return false;
}

QueryResult messageResult(bool success, std::string message) {
    QueryResult result;
    result.success = success;
    result.message = std::move(message);
    return result;
}

} // namespace

QueryExecutor::QueryExecutor(std::filesystem::path storageRoot)
    : storageManager_(storageRoot), wal_(storageRoot / "theCityCRDB.wal") {}

QueryResult QueryExecutor::execute(const Query &query) {
    return std::visit(
        [this](const auto &command) -> QueryResult {
            using Command = std::decay_t<decltype(command)>;
            if constexpr (std::is_same_v<Command, CreateDatabase>) {
                return executeCreateDatabase(command);
            } else if constexpr (std::is_same_v<Command, CreateTable>) {
                return executeCreateTable(command);
            } else if constexpr (std::is_same_v<Command, DropTable>) {
                return executeDropTable(command);
            } else if constexpr (std::is_same_v<Command, RenameTable>) {
                return executeRenameTable(command);
            } else if constexpr (std::is_same_v<Command, ListTables>) {
                return executeListTables();
            } else if constexpr (std::is_same_v<Command, Insert>) {
                return executeInsert(command);
            } else if constexpr (std::is_same_v<Command, Select>) {
                return executeSelect(command);
            } else if constexpr (std::is_same_v<Command, Update>) {
                return executeUpdate(command);
            } else if constexpr (std::is_same_v<Command, Delete>) {
                return executeDelete(command);
            } else if constexpr (std::is_same_v<Command, CreateIndex>) {
                return executeCreateIndex(command);
            } else if constexpr (std::is_same_v<Command, SaveDatabase>) {
                return executeSaveDatabase();
            } else if constexpr (std::is_same_v<Command, LoadDatabase>) {
                return executeLoadDatabase(command);
            } else if constexpr (std::is_same_v<Command, BeginTransaction>) {
                return executeBegin();
            } else if constexpr (std::is_same_v<Command, CommitTransaction>) {
                return executeCommit();
            } else if constexpr (std::is_same_v<Command, RollbackTransaction>) {
                return executeRollback();
            } else if constexpr (std::is_same_v<Command, Exit>) {
                return messageResult(true, "exit");
            }
        },
        query);
}

std::shared_ptr<Database> QueryExecutor::currentDatabase() const noexcept { return database_; }

QueryResult QueryExecutor::executeCreateDatabase(const CreateDatabase &command) {
    (void)wal_.append(WalOperation::CreateDatabase, command.name);
    database_ = std::make_shared<Database>(command.name);
    return messageResult(true, "created database " + command.name);
}

QueryResult QueryExecutor::executeCreateTable(const CreateTable &command) {
    if (!database_) {
        return messageResult(false, "no active database");
    }
    const bool created = database_->createTable(command.name, command.columns);
    if (created) {
        (void)wal_.append(WalOperation::CreateTable, command.name);
    }
    return messageResult(created,
                         created ? "created table " + command.name : "table already exists");
}

QueryResult QueryExecutor::executeDropTable(const DropTable &command) {
    if (!database_) {
        return messageResult(false, "no active database");
    }
    const bool dropped = database_->dropTable(command.name);
    if (dropped) {
        (void)wal_.append(WalOperation::DropTable, command.name);
    }
    return messageResult(dropped, dropped ? "dropped table " + command.name : "unknown table");
}

QueryResult QueryExecutor::executeRenameTable(const RenameTable &command) {
    if (!database_) {
        return messageResult(false, "no active database");
    }
    const bool renamed = database_->renameTable(command.oldName, command.newName);
    if (renamed) {
        (void)wal_.append(WalOperation::RenameTable, command.oldName + "->" + command.newName);
    }
    return messageResult(renamed, renamed ? "renamed table " + command.oldName : "rename failed");
}

QueryResult QueryExecutor::executeListTables() {
    if (!database_) {
        return messageResult(false, "no active database");
    }
    QueryResult result;
    result.message = "listed tables";
    result.columns = {"table"};
    for (const auto &name : database_->listTables()) {
        result.rows.push_back({Value{name}});
    }
    return result;
}

QueryResult QueryExecutor::executeInsert(const Insert &command) {
    auto table = requireTable(command.table);
    table->insert(command.values);
    (void)wal_.append(WalOperation::Insert, command.table);
    return messageResult(true, "inserted 1 row");
}

QueryResult QueryExecutor::executeSelect(const Select &command) {
    auto table = requireTable(command.table);

    std::vector<std::string> columns;
    const auto projection = resolveProjection(command, *table, columns);
    auto rows = collectRows(command, *table);

    if (command.orderBy) {
        const auto orderIndex = table->columnIndex(command.orderBy->column);
        if (!orderIndex) {
            throw std::runtime_error("unknown ORDER BY column");
        }
        std::ranges::sort(rows, [&](const Row &left, const Row &right) {
            if (command.orderBy->ascending) {
                return left[*orderIndex] < right[*orderIndex];
            }
            return right[*orderIndex] < left[*orderIndex];
        });
    }

    QueryResult result{true, "selected rows", std::move(columns), {}};
    for (const auto &row : rows) {
        Row projected;
        projected.reserve(projection.size());
        for (const auto index : projection) {
            projected.push_back(row[index]);
        }
        result.rows.push_back(std::move(projected));
        if (command.limit && result.rows.size() >= *command.limit) {
            break;
        }
    }
    return result;
}

QueryResult QueryExecutor::executeUpdate(const Update &command) {
    auto table = requireTable(command.table);
    const auto target = table->columnIndex(command.column);
    if (!target) {
        throw std::runtime_error("unknown update column");
    }

    std::size_t count = 0;
    const auto snapshot = table->rowsSnapshot();
    for (RowId rowId = 0; rowId < snapshot.size(); ++rowId) {
        if (command.where && !matches(snapshot[rowId], *table, *command.where)) {
            continue;
        }
        if (table->update(rowId, *target, command.value)) {
            (void)wal_.append(WalOperation::Update, command.table + "." + command.column);
            ++count;
        }
    }
    return messageResult(true, "updated " + std::to_string(count) + " row(s)");
}

QueryResult QueryExecutor::executeDelete(const Delete &command) {
    auto table = requireTable(command.table);
    std::size_t count = 0;
    const auto snapshot = table->rowsSnapshot();
    for (RowId rowId = snapshot.size(); rowId > 0; --rowId) {
        const RowId actual = rowId - 1;
        if (command.where && !matches(snapshot[actual], *table, *command.where)) {
            continue;
        }
        if (table->erase(actual)) {
            (void)wal_.append(WalOperation::Delete, command.table);
            ++count;
        }
    }
    return messageResult(true, "deleted " + std::to_string(count) + " row(s)");
}

QueryResult QueryExecutor::executeCreateIndex(const CreateIndex &command) {
    auto table = requireTable(command.table);
    const bool created = table->createIndex(command.name, command.column);
    if (created) {
        (void)wal_.append(WalOperation::CreateIndex, command.table + "." + command.column);
    }
    return messageResult(created,
                         created ? "created index " + command.name : "index creation failed");
}

QueryResult QueryExecutor::executeSaveDatabase() {
    if (!database_) {
        return messageResult(false, "no active database");
    }
    (void)wal_.append(WalOperation::SaveDatabase, database_->name());
    storageManager_.saveDatabase(*database_);
    return messageResult(true, "saved database " + database_->name());
}

QueryResult QueryExecutor::executeLoadDatabase(const LoadDatabase &command) {
    if (command.name) {
        database_ = storageManager_.loadDatabase(*command.name);
    } else if (database_) {
        database_ = storageManager_.loadDatabase(database_->name());
    } else {
        database_ = storageManager_.loadFirstDatabase();
    }
    transactionSnapshot_.reset();
    return messageResult(true, "loaded database " + database_->name());
}

QueryResult QueryExecutor::executeBegin() {
    if (!database_) {
        return messageResult(false, "no active database");
    }
    if (transactionSnapshot_) {
        return messageResult(false, "transaction already active");
    }
    activeTransaction_ = transactionManager_.begin().id;
    transactionSnapshot_ = database_->clone();
    return messageResult(true, "began transaction");
}

QueryResult QueryExecutor::executeCommit() {
    if (!transactionSnapshot_) {
        return messageResult(false, "no active transaction");
    }
    if (!activeTransaction_) {
        return messageResult(false, "transaction state is inconsistent");
    }
    transactionManager_.commit(*activeTransaction_);
    activeTransaction_.reset();
    transactionSnapshot_.reset();
    return messageResult(true, "committed transaction");
}

QueryResult QueryExecutor::executeRollback() {
    if (!transactionSnapshot_) {
        return messageResult(false, "no active transaction");
    }
    if (!activeTransaction_) {
        return messageResult(false, "transaction state is inconsistent");
    }
    transactionManager_.rollback(*activeTransaction_);
    activeTransaction_.reset();
    database_ = transactionSnapshot_;
    transactionSnapshot_.reset();
    return messageResult(true, "rolled back transaction");
}

std::vector<std::size_t> QueryExecutor::resolveProjection(const Select &command, const Table &table,
                                                          std::vector<std::string> &columns) const {
    std::vector<std::size_t> projection;
    if (command.columns.size() == 1 && command.columns.front() == "*") {
        for (std::size_t i = 0; i < table.schema().size(); ++i) {
            projection.push_back(i);
            columns.push_back(table.schema()[i].name);
        }
        return projection;
    }

    for (const auto &column : command.columns) {
        auto index = table.columnIndex(column);
        if (!index) {
            throw std::runtime_error("unknown selected column");
        }
        projection.push_back(*index);
        columns.push_back(column);
    }
    return projection;
}

std::vector<Row> QueryExecutor::collectRows(const Select &command, const Table &table) const {
    const auto plan = planner_.planSelect(command, table);
    if (command.where && plan.accessPath == AccessPath::HashIndexLookup) {
        if (auto rowIds = table.indexedLookup(command.where->column, command.where->value)) {
            return table.rowsById(*rowIds);
        }
    }
    if (command.where && plan.accessPath == AccessPath::OrderedIndexRange) {
        if (auto rowIds = table.orderedLookup(command.where->column, command.where->op,
                                              command.where->value)) {
            return table.rowsById(*rowIds);
        }
    }

    std::vector<Row> rows;
    const auto snapshot = table.rowsSnapshot();
    for (const auto &row : snapshot) {
        if (command.where && !matches(row, table, *command.where)) {
            continue;
        }
        rows.push_back(row);
    }
    return rows;
}

bool QueryExecutor::matches(const Row &row, const Table &table, const Predicate &predicate) const {
    const auto index = table.columnIndex(predicate.column);
    if (!index) {
        throw std::runtime_error("unknown predicate column");
    }
    return compare(row[*index], predicate.op, predicate.value);
}

std::shared_ptr<Table> QueryExecutor::requireTable(std::string_view tableName) const {
    if (!database_) {
        throw std::runtime_error("no active database");
    }
    auto table = database_->table(tableName);
    if (!table) {
        throw std::runtime_error("unknown table");
    }
    return table;
}

} // namespace theCityCRDB
