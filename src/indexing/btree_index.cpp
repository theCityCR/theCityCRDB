#include "theCityCRDB/indexing/btree_index.hpp"

#include <algorithm>
#include <mutex>

namespace theCityCRDB {

void BTreeIndex::insert(const Value &key, RowId rowId) {
    std::unique_lock lock{mutex_};
    entries_[key].push_back(rowId);
}

void BTreeIndex::remove(const Value &key, RowId rowId) {
    std::unique_lock lock{mutex_};
    auto it = entries_.find(key);
    if (it == entries_.end()) {
        return;
    }
    std::erase(it->second, rowId);
    if (it->second.empty()) {
        entries_.erase(it);
    }
}

void BTreeIndex::clear() {
    std::unique_lock lock{mutex_};
    entries_.clear();
}

std::vector<RowId> BTreeIndex::find(const Value &key) const {
    std::shared_lock lock{mutex_};
    auto it = entries_.find(key);
    if (it == entries_.end()) {
        return {};
    }
    return it->second;
}

std::vector<RowId> BTreeIndex::lessThan(const Value &key) const {
    std::shared_lock lock{mutex_};
    std::vector<RowId> result;
    for (auto it = entries_.begin(); it != entries_.lower_bound(key); ++it) {
        result.insert(result.end(), it->second.begin(), it->second.end());
    }
    return result;
}

std::vector<RowId> BTreeIndex::greaterThan(const Value &key) const {
    std::shared_lock lock{mutex_};
    std::vector<RowId> result;
    for (auto it = entries_.upper_bound(key); it != entries_.end(); ++it) {
        result.insert(result.end(), it->second.begin(), it->second.end());
    }
    return result;
}

std::size_t BTreeIndex::size() const {
    std::shared_lock lock{mutex_};
    return entries_.size();
}

} // namespace theCityCRDB
