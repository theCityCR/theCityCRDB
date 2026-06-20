#pragma once

#include "atlasdb/storage/row.hpp"

#include <map>
#include <shared_mutex>
#include <vector>

namespace atlasdb {

class HashIndex {
public:
    void insert(const Value& key, RowId rowId);
    void remove(const Value& key, RowId rowId);
    [[nodiscard]] std::vector<RowId> find(const Value& key) const;
    [[nodiscard]] std::size_t size() const;

private:
    std::map<Value, std::vector<RowId>> entries_;
    mutable std::shared_mutex mutex_;
};

}  // namespace atlasdb
