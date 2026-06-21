#include "theCityCRDB/concurrency/lock_manager.hpp"

namespace theCityCRDB {

std::shared_lock<std::shared_mutex> LockManager::acquireRead() const {
    return std::shared_lock<std::shared_mutex>{mutex_};
}

std::unique_lock<std::shared_mutex> LockManager::acquireWrite() const {
    return std::unique_lock<std::shared_mutex>{mutex_};
}

} // namespace theCityCRDB
