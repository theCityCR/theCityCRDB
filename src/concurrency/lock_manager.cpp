#include "atlasdb/concurrency/lock_manager.hpp"

namespace atlasdb {

std::shared_lock<std::shared_mutex> LockManager::acquireRead() {
    return std::shared_lock<std::shared_mutex>{mutex_};
}

std::unique_lock<std::shared_mutex> LockManager::acquireWrite() {
    return std::unique_lock<std::shared_mutex>{mutex_};
}

}  // namespace atlasdb
