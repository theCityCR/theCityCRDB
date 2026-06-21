#include "theCityCRDB/storage/row_store.hpp"

namespace theCityCRDB {

RowId VectorRowStore::append(Row row) {
    rows_.push_back(std::move(row));
    return rows_.size() - 1;
}

bool VectorRowStore::erase(RowId rowId) {
    if (rowId >= rows_.size()) {
        return false;
    }
    rows_.erase(rows_.begin() + static_cast<std::ptrdiff_t>(rowId));
    return true;
}

bool VectorRowStore::update(RowId rowId, Row row) {
    if (rowId >= rows_.size()) {
        return false;
    }
    rows_[rowId] = std::move(row);
    return true;
}

const Row *VectorRowStore::get(RowId rowId) const {
    if (rowId >= rows_.size()) {
        return nullptr;
    }
    return &rows_[rowId];
}

std::vector<Row> VectorRowStore::snapshot() const { return rows_; }

std::vector<Row> VectorRowStore::rowsById(std::span<const RowId> rowIds) const {
    std::vector<Row> rows;
    rows.reserve(rowIds.size());
    for (const auto rowId : rowIds) {
        if (const auto *row = get(rowId); row != nullptr) {
            rows.push_back(*row);
        }
    }
    return rows;
}

std::size_t VectorRowStore::size() const noexcept { return rows_.size(); }

void VectorRowStore::replaceRows(std::vector<Row> rows) { rows_ = std::move(rows); }

std::unique_ptr<RowStore> makeVectorRowStore() { return std::make_unique<VectorRowStore>(); }

} // namespace theCityCRDB
