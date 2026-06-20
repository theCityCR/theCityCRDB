#pragma once

#include "atlasdb/common/value.hpp"
#include "atlasdb/storage/row.hpp"

#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <vector>

namespace atlasdb {

class Table {
public:
    Table(std::string name, std::vector<Column> schema);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] std::span<const Column> schema() const noexcept;
    [[nodiscard]] std::optional<std::size_t> columnIndex(std::string_view column) const;
    [[nodiscard]] std::vector<Row> rowsSnapshot() const;
    [[nodiscard]] std::size_t rowCount() const;

    RowId insert(Row row);
    bool erase(RowId rowId);
    bool update(RowId rowId, std::size_t columnIndex, Value value);

private:
    void validateRow(const Row& row) const;

    std::string name_;
    std::vector<Column> schema_;
    std::vector<Row> rows_;
    mutable std::shared_mutex mutex_;
};

}  // namespace atlasdb
