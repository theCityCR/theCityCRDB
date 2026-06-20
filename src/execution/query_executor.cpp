#include "atlasdb/execution/query_executor.hpp"

#include <algorithm>
#include <stdexcept>

namespace atlasdb {
namespace {

bool compare(const Value& left, ComparisonOperator op, const Value& right) {
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

}  // namespace

QueryResult QueryExecutor::execute(const Query& query) {
    return std::visit(
        [this](const auto& command) -> QueryResult {
            using Command = std::decay_t<decltype(command)>;
            if constexpr (std::is_same_v<Command, CreateDatabase>) {
                return executeCreateDatabase(command);
            } else if constexpr (std::is_same_v<Command, CreateTable>) {
                return executeCreateTable(command);
            } else if constexpr (std::is_same_v<Command, Insert>) {
                return executeInsert(command);
            } else if constexpr (std::is_same_v<Command, Select>) {
                return executeSelect(command);
            } else if constexpr (std::is_same_v<Command, Update>) {
                return executeUpdate(command);
            } else if constexpr (std::is_same_v<Command, Delete>) {
                return executeDelete(command);
            } else if constexpr (std::is_same_v<Command, CreateIndex>) {
                return {true, "index creation is scaffolded for the index manager"};
            } else if constexpr (std::is_same_v<Command, SaveDatabase>) {
                return {true, "save is scaffolded for the persistence layer"};
            } else if constexpr (std::is_same_v<Command, LoadDatabase>) {
                return {true, "load is scaffolded for the persistence layer"};
            } else if constexpr (std::is_same_v<Command, Exit>) {
                return {true, "exit"};
            }
        },
        query);
}

std::shared_ptr<Database> QueryExecutor::currentDatabase() const noexcept {
    return database_;
}

QueryResult QueryExecutor::executeCreateDatabase(const CreateDatabase& command) {
    database_ = std::make_shared<Database>(command.name);
    return {true, "created database " + command.name};
}

QueryResult QueryExecutor::executeCreateTable(const CreateTable& command) {
    if (!database_) {
        return {false, "no active database"};
    }
    const bool created = database_->createTable(command.name, command.columns);
    return {created, created ? "created table " + command.name : "table already exists"};
}

QueryResult QueryExecutor::executeInsert(const Insert& command) {
    auto table = requireTable(command.table);
    table->insert(command.values);
    return {true, "inserted 1 row"};
}

QueryResult QueryExecutor::executeSelect(const Select& command) {
    auto table = requireTable(command.table);
    const auto snapshot = table->rowsSnapshot();

    std::vector<std::size_t> projection;
    std::vector<std::string> columns;
    if (command.columns.size() == 1 && command.columns.front() == "*") {
        for (std::size_t i = 0; i < table->schema().size(); ++i) {
            projection.push_back(i);
            columns.push_back(table->schema()[i].name);
        }
    } else {
        for (const auto& column : command.columns) {
            auto index = table->columnIndex(column);
            if (!index) {
                throw std::runtime_error("unknown selected column");
            }
            projection.push_back(*index);
            columns.push_back(column);
        }
    }

    QueryResult result{true, "selected rows", std::move(columns), {}};
    for (const auto& row : snapshot) {
        if (command.where && !matches(row, *table, *command.where)) {
            continue;
        }
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

QueryResult QueryExecutor::executeUpdate(const Update& command) {
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
            ++count;
        }
    }
    return {true, "updated " + std::to_string(count) + " row(s)"};
}

QueryResult QueryExecutor::executeDelete(const Delete& command) {
    auto table = requireTable(command.table);
    std::size_t count = 0;
    const auto snapshot = table->rowsSnapshot();
    for (RowId rowId = snapshot.size(); rowId > 0; --rowId) {
        const RowId actual = rowId - 1;
        if (command.where && !matches(snapshot[actual], *table, *command.where)) {
            continue;
        }
        if (table->erase(actual)) {
            ++count;
        }
    }
    return {true, "deleted " + std::to_string(count) + " row(s)"};
}

bool QueryExecutor::matches(const Row& row, const Table& table, const Predicate& predicate) const {
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

}  // namespace atlasdb
