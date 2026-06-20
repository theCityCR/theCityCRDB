#include "theCityCRDB/storage/database.hpp"

#include <gtest/gtest.h>

namespace theCityCRDB {

TEST(StorageTests, CreatesTableAndInsertsTypedRows) {
    Database database{"company"};
    ASSERT_TRUE(database.createTable("Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}));

    auto table = database.table("Employees");
    ASSERT_NE(table, nullptr);
    table->insert({Value{1}, Value{std::string{"Alice"}}});

    EXPECT_EQ(table->rowCount(), 1U);
}

TEST(StorageTests, RejectsRowsWithWrongShape) {
    Table table{"Employees", {{"id", ColumnType::Int}}};

    EXPECT_THROW(table.insert({Value{1}, Value{2}}), std::invalid_argument);
}

}  // namespace theCityCRDB
