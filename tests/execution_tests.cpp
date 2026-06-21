#include "theCityCRDB/execution/query_executor.hpp"
#include "theCityCRDB/parser/parser.hpp"
#include "theCityCRDB/persistence/write_ahead_log.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>

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

TEST(ExecutionTests, OrdersLimitsAndManagesTables) {
    Parser parser;
    QueryExecutor executor;

    ASSERT_TRUE(executor.execute(parser.parse("CREATE DATABASE company;")).success);
    ASSERT_TRUE(executor.execute(parser.parse("CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);")).success);
    ASSERT_TRUE(executor.execute(parser.parse("INSERT INTO Employees VALUES (1, \"Alice\", 120000.0);")).success);
    ASSERT_TRUE(executor.execute(parser.parse("INSERT INTO Employees VALUES (2, \"Bob\", 90000.0);")).success);

    auto ordered = executor.execute(parser.parse("SELECT name FROM Employees ORDER BY salary ASC LIMIT 1;"));
    ASSERT_EQ(ordered.rows.size(), 1U);
    EXPECT_EQ(ordered.rows.front().front(), Value{std::string{"Bob"}});

    auto listed = executor.execute(parser.parse("LIST TABLES;"));
    ASSERT_EQ(listed.rows.size(), 1U);
    EXPECT_EQ(listed.rows.front().front(), Value{std::string{"Employees"}});

    EXPECT_TRUE(executor.execute(parser.parse("RENAME TABLE Employees TO Staff;")).success);
    EXPECT_TRUE(executor.execute(parser.parse("DROP TABLE Staff;")).success);
}

TEST(ExecutionTests, RollsBackTransactions) {
    Parser parser;
    QueryExecutor executor;

    ASSERT_TRUE(executor.execute(parser.parse("CREATE DATABASE company;")).success);
    ASSERT_TRUE(executor.execute(parser.parse("CREATE TABLE Employees (id INT);")).success);
    ASSERT_TRUE(executor.execute(parser.parse("BEGIN;")).success);
    ASSERT_TRUE(executor.execute(parser.parse("INSERT INTO Employees VALUES (1);")).success);
    ASSERT_TRUE(executor.execute(parser.parse("ROLLBACK;")).success);

    auto result = executor.execute(parser.parse("SELECT * FROM Employees;"));
    EXPECT_TRUE(result.rows.empty());
}

TEST(ExecutionTests, SavesAndLoadsDatabase) {
    const auto root = std::filesystem::temp_directory_path() /
                      ("theCityCRDB_test_" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Parser parser;

    {
        QueryExecutor executor{root};
        ASSERT_TRUE(executor.execute(parser.parse("CREATE DATABASE company;")).success);
        ASSERT_TRUE(executor.execute(parser.parse("CREATE TABLE Employees (id INT, name STRING);")).success);
        ASSERT_TRUE(executor.execute(parser.parse("CREATE INDEX idx_id ON Employees(id);")).success);
        ASSERT_TRUE(executor.execute(parser.parse("INSERT INTO Employees VALUES (1, \"Alice\");")).success);
        ASSERT_TRUE(executor.execute(parser.parse("SAVE DATABASE;")).success);
    }

    QueryExecutor loaded{root};
    ASSERT_TRUE(loaded.execute(parser.parse("LOAD DATABASE company;")).success);
    auto result = loaded.execute(parser.parse("SELECT name FROM Employees WHERE id = 1;"));
    ASSERT_EQ(result.rows.size(), 1U);
    EXPECT_EQ(result.rows.front().front(), Value{std::string{"Alice"}});

    std::filesystem::remove_all(root);
}

TEST(ExecutionTests, FailedMetadataOperationsDoNotPolluteWal) {
    const auto root = std::filesystem::temp_directory_path() /
                      ("theCityCRDB_failed_wal_test_" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Parser parser;
    QueryExecutor executor{root};

    ASSERT_TRUE(executor.execute(parser.parse("CREATE DATABASE company;")).success);
    ASSERT_TRUE(executor.execute(parser.parse("CREATE TABLE Employees (id INT);")).success);
    EXPECT_FALSE(executor.execute(parser.parse("CREATE TABLE Employees (id INT);")).success);
    EXPECT_FALSE(executor.execute(parser.parse("DROP TABLE Missing;")).success);

    WriteAheadLog wal{root / "theCityCRDB.wal"};
    const auto records = wal.readAll();
    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].operation, WalOperation::CreateDatabase);
    EXPECT_EQ(records[1].operation, WalOperation::CreateTable);

    std::filesystem::remove_all(root);
}

TEST(ExecutionTests, FailedInsertDoesNotPolluteWal) {
    const auto root = std::filesystem::temp_directory_path() /
                      ("theCityCRDB_failed_insert_wal_test_" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Parser parser;
    QueryExecutor executor{root};

    ASSERT_TRUE(executor.execute(parser.parse("CREATE DATABASE company;")).success);
    ASSERT_TRUE(executor.execute(parser.parse("CREATE TABLE Employees (id INT);")).success);
    EXPECT_THROW((void)executor.execute(parser.parse("INSERT INTO Employees VALUES (1, \"extra\");")),
                 std::invalid_argument);

    WriteAheadLog wal{root / "theCityCRDB.wal"};
    const auto records = wal.readAll();
    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].operation, WalOperation::CreateDatabase);
    EXPECT_EQ(records[1].operation, WalOperation::CreateTable);

    std::filesystem::remove_all(root);
}

}  // namespace theCityCRDB
