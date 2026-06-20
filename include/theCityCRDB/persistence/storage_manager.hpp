#pragma once

#include "theCityCRDB/storage/database.hpp"

#include <filesystem>

namespace theCityCRDB {

class StorageManager {
public:
    explicit StorageManager(std::filesystem::path root);

    void saveMetadata(const Database& database) const;
    [[nodiscard]] bool metadataExists(std::string_view databaseName) const;

private:
    std::filesystem::path root_;
};

}  // namespace theCityCRDB
