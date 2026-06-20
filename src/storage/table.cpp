#include "atlasdb/storage/table.hpp"

#include <algorithm>
#include <stdexcept>

namespace atlasdb {

Table::Table(std::string name, std::vector<Column> schema)
    : name_(std::move(name)), schema_(std::move(schema)) {
    if (name_.empty()) {
        throw std::invalid_argument("table name cannot be empty");
    }
    if (schema_.empty()) {
        throw std::invalid_argument("table schema cannot be empty");
    }
}

const std::string& Table::name() const noexcept {
    return name_;
}

std::span<const Column> Table::schema() const noexcept {
    return schema_;
}

std::optional<std::size_t> Table::columnIndex(std::string_view column) const {
    auto it = std::ranges::find_if(schema_, [column](const Column& item) {
        return item.name == column;
    });
    if (it == schema_.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(schema_.begin(), it));
}

std::vector<Row> Table::rowsSnapshot() const {
    std::shared_lock lock{mutex_};
    return rows_;
}

std::size_t Table::rowCount() const {
    std::shared_lock lock{mutex_};
    return rows_.size();
}

RowId Table::insert(Row row) {
    validateRow(row);
    std::unique_lock lock{mutex_};
    rows_.push_back(std::move(row));
    return rows_.size() - 1;
}

bool Table::erase(RowId rowId) {
    std::unique_lock lock{mutex_};
    if (rowId >= rows_.size()) {
        return false;
    }
    rows_.erase(rows_.begin() + static_cast<std::ptrdiff_t>(rowId));
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
    return true;
}

void Table::validateRow(const Row& row) const {
    if (row.size() != schema_.size()) {
        throw std::invalid_argument("row width does not match table schema");
    }
    for (std::size_t i = 0; i < row.size(); ++i) {
        if (row[i].type() != schema_[i].type) {
            throw std::invalid_argument("row value does not match column type");
        }
    }
}

}  // namespace atlasdb
