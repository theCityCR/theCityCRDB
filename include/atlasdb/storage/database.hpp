#pragma once

#include "atlasdb/storage/table.hpp"

#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

namespace atlasdb {

class Database {
public:
    explicit Database(std::string name);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] bool createTable(std::string name, std::vector<Column> schema);
    [[nodiscard]] bool dropTable(std::string_view name);
    [[nodiscard]] bool renameTable(std::string_view oldName, std::string newName);
    [[nodiscard]] std::shared_ptr<Table> table(std::string_view name) const;
    [[nodiscard]] std::vector<std::string> listTables() const;

private:
    std::string name_;
    std::map<std::string, std::shared_ptr<Table>, std::less<>> tables_;
    mutable std::shared_mutex mutex_;
};

}  // namespace atlasdb
