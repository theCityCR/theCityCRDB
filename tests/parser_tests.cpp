#include "theCityCRDB/parser/parser.hpp"

#include <gtest/gtest.h>

namespace theCityCRDB {

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

TEST(ParserTests, ParsesOrderByAndTransactionCommands) {
    Parser parser;

    auto select = parser.parse("SELECT * FROM Employees ORDER BY salary DESC LIMIT 2;");
    ASSERT_TRUE(std::holds_alternative<Select>(select));
    const auto& command = std::get<Select>(select);
    ASSERT_TRUE(command.orderBy.has_value());
    EXPECT_EQ(command.orderBy->column, "salary");
    EXPECT_FALSE(command.orderBy->ascending);

    EXPECT_TRUE(std::holds_alternative<BeginTransaction>(parser.parse("BEGIN;")));
    EXPECT_TRUE(std::holds_alternative<CommitTransaction>(parser.parse("COMMIT;")));
    EXPECT_TRUE(std::holds_alternative<RollbackTransaction>(parser.parse("ROLLBACK;")));
}

TEST(ParserTests, ParsesTableManagementCommands) {
    Parser parser;

    EXPECT_TRUE(std::holds_alternative<DropTable>(parser.parse("DROP TABLE Employees;")));
    EXPECT_TRUE(std::holds_alternative<RenameTable>(parser.parse("RENAME TABLE Employees TO Staff;")));
    EXPECT_TRUE(std::holds_alternative<ListTables>(parser.parse("LIST TABLES;")));
}

}  // namespace theCityCRDB
