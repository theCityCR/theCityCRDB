#include "atlasdb/indexing/hash_index.hpp"

#include <algorithm>
#include <mutex>

namespace atlasdb {

void HashIndex::insert(const Value& key, RowId rowId) {
    std::unique_lock lock{mutex_};
    entries_[key].push_back(rowId);
}

void HashIndex::remove(const Value& key, RowId rowId) {
    std::unique_lock lock{mutex_};
    auto it = entries_.find(key);
    if (it == entries_.end()) {
        return;
    }
    auto& rowIds = it->second;
    std::erase(rowIds, rowId);
    if (rowIds.empty()) {
        entries_.erase(it);
    }
}

std::vector<RowId> HashIndex::find(const Value& key) const {
    std::shared_lock lock{mutex_};
    auto it = entries_.find(key);
    if (it == entries_.end()) {
        return {};
    }
    return it->second;
}

std::size_t HashIndex::size() const {
    std::shared_lock lock{mutex_};
    return entries_.size();
}

}  // namespace atlasdb
