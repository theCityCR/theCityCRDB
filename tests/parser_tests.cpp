#include "atlasdb/parser/parser.hpp"

#include <gtest/gtest.h>

namespace atlasdb {

TEST(ParserTests, ParsesCreateTable) {
    Parser parser;
    auto query = parser.parse("CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);");

    ASSERT_TRUE(std::holds_alternative<CreateTable>(query));
    const auto& command = std::get<CreateTable>(query);
    EXPECT_EQ(command.name, "Employees");
    ASSERT_EQ(command.columns.size(), 3U);
    EXPECT_EQ(command.columns[0].type, ColumnType::Int);
    EXPECT_EQ(command.columns[1].type, ColumnType::String);
    EXPECT_EQ(command.columns[2].type, ColumnType::Double);
}

TEST(ParserTests, ParsesSelectWithPredicateAndLimit) {
    Parser parser;
    auto query = parser.parse("SELECT name FROM Employees WHERE salary > 100000.0 LIMIT 5;");

    ASSERT_TRUE(std::holds_alternative<Select>(query));
    const auto& command = std::get<Select>(query);
    EXPECT_EQ(command.table, "Employees");
    ASSERT_EQ(command.columns.size(), 1U);
    EXPECT_EQ(command.columns.front(), "name");
    ASSERT_TRUE(command.where.has_value());
    EXPECT_EQ(command.where->column, "salary");
    ASSERT_TRUE(command.limit.has_value());
    EXPECT_EQ(*command.limit, 5U);
}

}  // namespace atlasdb
