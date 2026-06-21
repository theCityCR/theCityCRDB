#pragma once

#include "theCityCRDB/storage/database.hpp"

#include <filesystem>
#include <memory>
#include <string_view>

namespace theCityCRDB {

class StorageManager {
  public:
    explicit StorageManager(std::filesystem::path root);

    void saveDatabase(const Database &database) const;
    [[nodiscard]] std::shared_ptr<Database> loadDatabase(std::string_view databaseName) const;
    [[nodiscard]] std::shared_ptr<Database> loadFirstDatabase() const;
    [[nodiscard]] bool metadataExists(std::string_view databaseName) const;

  private:
    std::filesystem::path root_;
};

} // namespace theCityCRDB
