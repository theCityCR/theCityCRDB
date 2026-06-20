#include "atlasdb/storage/database.hpp"

#include <stdexcept>

namespace atlasdb {

Database::Database(std::string name) : name_(std::move(name)) {
    if (name_.empty()) {
        throw std::invalid_argument("database name cannot be empty");
    }
}

const std::string& Database::name() const noexcept {
    return name_;
}

bool Database::createTable(std::string name, std::vector<Column> schema) {
    std::unique_lock lock{mutex_};
    auto [_, inserted] = tables_.try_emplace(name, std::make_shared<Table>(name, std::move(schema)));
    return inserted;
}

bool Database::dropTable(std::string_view name) {
    std::unique_lock lock{mutex_};
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return false;
    }
    tables_.erase(it);
    return true;
}

bool Database::renameTable(std::string_view oldName, std::string newName) {
    std::unique_lock lock{mutex_};
    auto it = tables_.find(oldName);
    if (it == tables_.end() || tables_.contains(newName)) {
        return false;
    }
    auto oldTable = it->second;
    tables_.erase(it);
    std::vector<Column> schema{oldTable->schema().begin(), oldTable->schema().end()};
    auto replacement = std::make_shared<Table>(newName, std::move(schema));
    for (const auto& row : oldTable->rowsSnapshot()) {
        replacement->insert(row);
    }
    tables_.emplace(std::move(newName), std::move(replacement));
    return true;
}

std::shared_ptr<Table> Database::table(std::string_view name) const {
    std::shared_lock lock{mutex_};
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return {};
    }
    return it->second;
}

std::vector<std::string> Database::listTables() const {
    std::shared_lock lock{mutex_};
    std::vector<std::string> names;
    names.reserve(tables_.size());
    for (const auto& [name, _] : tables_) {
        names.push_back(name);
    }
    return names;
}

}  // namespace atlasdb
