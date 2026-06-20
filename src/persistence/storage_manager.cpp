#include "atlasdb/persistence/storage_manager.hpp"

#include <fstream>
#include <stdexcept>

namespace atlasdb {
namespace {

constexpr std::string_view kMagic = "ATLASDB";
constexpr std::uint32_t kVersion = 1;

}  // namespace

StorageManager::StorageManager(std::filesystem::path root) : root_(std::move(root)) {}

void StorageManager::saveMetadata(const Database& database) const {
    std::filesystem::create_directories(root_);
    const auto path = root_ / (database.name() + ".adb");
    std::ofstream out{path, std::ios::binary};
    if (!out) {
        throw std::runtime_error("failed to open database metadata for writing");
    }
    out.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
    out.write(reinterpret_cast<const char*>(&kVersion), sizeof(kVersion));
    const auto tables = database.listTables();
    const auto tableCount = static_cast<std::uint64_t>(tables.size());
    out.write(reinterpret_cast<const char*>(&tableCount), sizeof(tableCount));
}

bool StorageManager::metadataExists(std::string_view databaseName) const {
    return std::filesystem::exists(root_ / (std::string{databaseName} + ".adb"));
}

}  // namespace atlasdb
