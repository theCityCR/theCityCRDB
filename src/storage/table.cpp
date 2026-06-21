#include "theCityCRDB/storage/table.hpp"

#include <algorithm>
#include <mutex>
#include <stdexcept>

namespace theCityCRDB {

Table::Table(std::string name, std::vector<Column> schema)
    : name_(std::move(name)), schema_(std::move(schema)) {
    if (name_.empty()) {
        throw std::invalid_argument("table name cannot be empty");
    }
    if (schema_.empty()) {
        throw std::invalid_argument("table schema cannot be empty");
    }
}

const std::string &Table::name() const noexcept { return name_; }

std::span<const Column> Table::schema() const noexcept { return schema_; }

std::optional<std::size_t> Table::columnIndex(std::string_view column) const {
    auto it =
        std::ranges::find_if(schema_, [column](const Column &item) { return item.name == column; });
    if (it == schema_.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(schema_.begin(), it));
}

std::vector<Row> Table::rowsSnapshot() const {
    std::shared_lock lock{mutex_};
    return rows_;
}

std::vector<Row> Table::rowsById(std::span<const RowId> rowIds) const {
    std::shared_lock lock{mutex_};
    std::vector<Row> rows;
    rows.reserve(rowIds.size());
    for (const auto rowId : rowIds) {
        if (rowId < rows_.size()) {
            rows.push_back(rows_[rowId]);
        }
    }
    return rows;
}

std::size_t Table::rowCount() const {
    std::shared_lock lock{mutex_};
    return rows_.size();
}

std::vector<RowId> Table::findIndexed(std::string_view column, const Value &value) const {
    auto result = indexedLookup(column, value);
    if (!result) {
        return {};
    }
    return *result;
}

std::optional<std::vector<RowId>> Table::indexedLookup(std::string_view column,
                                                       const Value &value) const {
    std::shared_lock lock{mutex_};
    for (const auto &[indexName, columnIndex] : indexColumns_) {
        if (schema_[columnIndex].name == column) {
            return indexes_.at(indexName).find(value);
        }
    }
    return std::nullopt;
}

std::optional<std::vector<RowId>>
Table::orderedLookup(std::string_view column, ComparisonOperator op, const Value &value) const {
    std::shared_lock lock{mutex_};
    for (const auto &[indexName, columnIndex] : indexColumns_) {
        if (schema_[columnIndex].name != column) {
            continue;
        }
        if (op == ComparisonOperator::Equal) {
            return orderedIndexes_.at(indexName).find(value);
        }
        if (op == ComparisonOperator::Greater) {
            return orderedIndexes_.at(indexName).greaterThan(value);
        }
        if (op == ComparisonOperator::Less) {
            return orderedIndexes_.at(indexName).lessThan(value);
        }
    }
    return std::nullopt;
}

bool Table::hasIndex(std::string_view column) const {
    std::shared_lock lock{mutex_};
    return std::ranges::any_of(
        indexColumns_, [&](const auto &item) { return schema_[item.second].name == column; });
}

bool Table::hasOrderedIndex(std::string_view column) const { return hasIndex(column); }

std::vector<std::string> Table::listIndexes() const {
    std::shared_lock lock{mutex_};
    std::vector<std::string> names;
    names.reserve(indexes_.size());
    for (const auto &[name, _] : indexes_) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::pair<std::string, std::string>> Table::indexDefinitions() const {
    std::shared_lock lock{mutex_};
    std::vector<std::pair<std::string, std::string>> definitions;
    definitions.reserve(indexColumns_.size());
    for (const auto &[name, columnIndex] : indexColumns_) {
        definitions.emplace_back(name, schema_[columnIndex].name);
    }
    return definitions;
}

RowId Table::insert(Row row) {
    validateRow(row);
    std::unique_lock lock{mutex_};
    rows_.push_back(std::move(row));
    const RowId rowId = rows_.size() - 1;
    addRowToIndexes(rowId);
    return rowId;
}

bool Table::erase(RowId rowId) {
    std::unique_lock lock{mutex_};
    if (rowId >= rows_.size()) {
        return false;
    }
    rows_.erase(rows_.begin() + static_cast<std::ptrdiff_t>(rowId));
    rebuildIndexes();
    return true;
}

bool Table::update(RowId rowId, std::size_t index, Value value) {
    std::unique_lock lock{mutex_};
    if (rowId >= rows_.size() || index >= schema_.size()) {
        return false;
    }
    if (value.type() != schema_[index].type) {
        throw std::invalid_argument("updated value does not match column type");
    }
    rows_[rowId][index] = std::move(value);
    rebuildIndexes();
    return true;
}

bool Table::createIndex(std::string name, std::string column) {
    auto indexColumn = columnIndex(column);
    if (!indexColumn) {
        return false;
    }

    std::unique_lock lock{mutex_};
    if (indexes_.contains(name)) {
        return false;
    }
    indexColumns_.emplace(name, *indexColumn);
    indexes_.try_emplace(name);
    orderedIndexes_.try_emplace(name);
    for (RowId rowId = 0; rowId < rows_.size(); ++rowId) {
        indexes_.at(name).insert(rows_[rowId][*indexColumn], rowId);
        orderedIndexes_.at(name).insert(rows_[rowId][*indexColumn], rowId);
    }
    return true;
}

void Table::replaceRows(std::vector<Row> rows) {
    for (const auto &row : rows) {
        validateRow(row);
    }
    std::unique_lock lock{mutex_};
    rows_ = std::move(rows);
    rebuildIndexes();
}

void Table::validateRow(const Row &row) const {
    if (row.size() != schema_.size()) {
        throw std::invalid_argument("row width does not match table schema");
    }
    for (std::size_t i = 0; i < row.size(); ++i) {
        if (row[i].type() != schema_[i].type) {
            throw std::invalid_argument("row value does not match column type");
        }
    }
}

void Table::addRowToIndexes(RowId rowId) {
    for (auto &[name, index] : indexes_) {
        index.insert(rows_[rowId][indexColumns_.at(name)], rowId);
    }
    for (auto &[name, index] : orderedIndexes_) {
        index.insert(rows_[rowId][indexColumns_.at(name)], rowId);
    }
}

void Table::rebuildIndexes() {
    for (auto &[_, index] : indexes_) {
        index.clear();
    }
    for (auto &[_, index] : orderedIndexes_) {
        index.clear();
    }
    for (RowId rowId = 0; rowId < rows_.size(); ++rowId) {
        addRowToIndexes(rowId);
    }
}

} // namespace theCityCRDB
