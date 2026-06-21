#pragma once

#include "theCityCRDB/common/comparison_operator.hpp"
#include "theCityCRDB/common/value.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace theCityCRDB {

struct Predicate {
    std::string column;
    ComparisonOperator op;
    Value value;
};

struct CreateDatabase {
    std::string name;
};

struct CreateTable {
    std::string name;
    std::vector<Column> columns;
};

struct DropTable {
    std::string name;
};

struct RenameTable {
    std::string oldName;
    std::string newName;
};

struct ListTables {};

struct Insert {
    std::string table;
    std::vector<Value> values;
};

struct OrderBy {
    std::string column;
    bool ascending{true};
};

struct JoinClause {
    std::string table;
    std::string leftColumn;
    std::string rightColumn;
};

struct Select {
    std::string table;
    std::optional<JoinClause> join;
    std::vector<std::string> columns;
    std::optional<Predicate> where;
    std::optional<OrderBy> orderBy;
    std::optional<std::size_t> limit;
};

struct Update {
    std::string table;
    std::string column;
    Value value;
    std::optional<Predicate> where;
};

struct Delete {
    std::string table;
    std::optional<Predicate> where;
};

struct CreateIndex {
    std::string name;
    std::string table;
    std::string column;
};

struct SaveDatabase {};
struct LoadDatabase {
    std::optional<std::string> name;
};
struct BeginTransaction {};
struct CommitTransaction {};
struct RollbackTransaction {};
struct PrepareStatement {
    std::string name;
    std::string sql;
};
struct ExecutePrepared {
    std::string name;
    std::vector<Value> parameters;
};
struct Exit {};

using Query =
    std::variant<CreateDatabase, CreateTable, DropTable, RenameTable, ListTables, Insert, Select,
                 Update, Delete, CreateIndex, SaveDatabase, LoadDatabase, BeginTransaction,
                 CommitTransaction, RollbackTransaction, PrepareStatement, ExecutePrepared, Exit>;

} // namespace theCityCRDB
