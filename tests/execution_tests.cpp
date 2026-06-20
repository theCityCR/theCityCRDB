#include "theCityCRDB/execution/query_executor.hpp"
#include "theCityCRDB/parser/parser.hpp"

#include <gtest/gtest.h>

namespace theCityCRDB {

TEST(ExecutionTests, InsertsAndSelectsRows) {
    Parser parser;
    QueryExecutor executor;

    EXPECT_TRUE(executor.execute(parser.parse("CREATE DATABASE company;")).success);
    EXPECT_TRUE(executor.execute(parser.parse("CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);")).success);
    EXPECT_TRUE(executor.execute(parser.parse("INSERT INTO Employees VALUES (1, \"Alice\", 120000.0);")).success);

    auto result = executor.execute(parser.parse("SELECT name FROM Employees WHERE salary > 100000.0 LIMIT 1;"));
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.rows.size(), 1U);
    EXPECT_EQ(result.rows.front().front(), Value{std::string{"Alice"}});
}

}  // namespace theCityCRDB
