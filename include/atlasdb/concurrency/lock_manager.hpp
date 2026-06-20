#pragma once

#include <shared_mutex>

namespace atlasdb {

class LockManager {
public:
    [[nodiscard]] std::shared_lock<std::shared_mutex> acquireRead();
    [[nodiscard]] std::unique_lock<std::shared_mutex> acquireWrite();

private:
    std::shared_mutex mutex_;
};

}  // namespace atlasdb
