#include "theCityCRDB/storage/database.hpp"

#include <gtest/gtest.h>
#include <thread>

namespace theCityCRDB {

TEST(StorageTests, CreatesTableAndInsertsTypedRows) {
    Database database{"company"};
    ASSERT_TRUE(
        database.createTable("Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}));

    auto table = database.table("Employees");
    ASSERT_NE(table, nullptr);
    table->insert({Value{1}, Value{std::string{"Alice"}}});

    EXPECT_EQ(table->rowCount(), 1U);
}

TEST(StorageTests, RejectsRowsWithWrongShape) {
    Table table{"Employees", {{"id", ColumnType::Int}}};

    EXPECT_THROW(table.insert({Value{1}, Value{2}}), std::invalid_argument);
}

TEST(StorageTests, SupportsNullableColumns) {
    Table table{"Employees", {{"id", ColumnType::Int}, {"nickname", ColumnType::String, true}}};
    EXPECT_NO_THROW(table.insert({Value{1}, Value{}}));

    Table strict{"Employees", {{"id", ColumnType::Int}, {"nickname", ColumnType::String}}};
    EXPECT_THROW(strict.insert({Value{1}, Value{}}), std::invalid_argument);
}

TEST(StorageTests, MaintainsIndexesAcrossInsertUpdateAndDelete) {
    Table table{"Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}};
    ASSERT_TRUE(table.createIndex("idx_id", "id"));

    const auto first = table.insert({Value{1}, Value{std::string{"Alice"}}});
    table.insert({Value{2}, Value{std::string{"Bob"}}});
    EXPECT_EQ(table.findIndexed("id", Value{1}), std::vector<RowId>{first});

    ASSERT_TRUE(table.update(first, 0, Value{3}));
    EXPECT_TRUE(table.findIndexed("id", Value{1}).empty());
    EXPECT_EQ(table.findIndexed("id", Value{3}), std::vector<RowId>{first});

    ASSERT_TRUE(table.erase(first));
    EXPECT_TRUE(table.findIndexed("id", Value{3}).empty());
}

TEST(StorageTests, TracksRowVersionsAcrossTableMutations) {
    Table table{"Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}};

    const auto rowId = table.insert({Value{1}, Value{std::string{"Alice"}}});
    EXPECT_EQ(table.versionCount(rowId), 1U);

    ASSERT_TRUE(table.update(rowId, 1, Value{std::string{"Alicia"}}));
    EXPECT_EQ(table.versionCount(rowId), 2U);

    ASSERT_TRUE(table.erase(rowId));
    EXPECT_EQ(table.versionCount(rowId), 2U);
}

TEST(StorageTests, SupportsConcurrentInserts) {
    Table table{"Events", {{"id", ColumnType::Int}}};
    constexpr int threadCount = 4;
    constexpr int insertsPerThread = 250;

    std::vector<std::jthread> threads;
    for (int thread = 0; thread < threadCount; ++thread) {
        threads.emplace_back([thread, &table] {
            for (int i = 0; i < insertsPerThread; ++i) {
                table.insert({Value{thread * insertsPerThread + i}});
            }
        });
    }

    threads.clear();
    EXPECT_EQ(table.rowCount(), static_cast<std::size_t>(threadCount * insertsPerThread));
}

} // namespace theCityCRDB
