#include "theCityCRDB/transaction/mvcc_row_store.hpp"

#include <algorithm>

namespace theCityCRDB {

void MVCCRowStore::write(RowId rowId, Row row, TransactionId transactionId) {
    versions_[rowId].push_back({transactionId, std::nullopt, std::move(row)});
}

void MVCCRowStore::erase(RowId rowId, TransactionId transactionId) {
    auto it = versions_.find(rowId);
    if (it == versions_.end()) {
        return;
    }
    if (!it->second.empty()) {
        it->second.back().deletedBy = transactionId;
    }
}

std::optional<Row> MVCCRowStore::read(RowId rowId, TransactionId readerId) const {
    auto it = versions_.find(rowId);
    if (it == versions_.end()) {
        return std::nullopt;
    }
    for (auto version = it->second.rbegin(); version != it->second.rend(); ++version) {
        if (version->createdBy <= readerId &&
            (!version->deletedBy || *version->deletedBy > readerId)) {
            return version->row;
        }
    }
    return std::nullopt;
}

std::size_t MVCCRowStore::versionCount(RowId rowId) const {
    auto it = versions_.find(rowId);
    if (it == versions_.end()) {
        return 0;
    }
    return it->second.size();
}

} // namespace theCityCRDB
