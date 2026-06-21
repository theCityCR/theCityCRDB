#include "theCityCRDB/indexing/btree_index.hpp"
#include "theCityCRDB/persistence/write_ahead_log.hpp"
#include "theCityCRDB/planner/query_planner.hpp"
#include "theCityCRDB/storage/buffer_pool.hpp"
#include "theCityCRDB/storage/table.hpp"
#include "theCityCRDB/transaction/mvcc_row_store.hpp"
#include "theCityCRDB/transaction/transaction_manager.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>

namespace theCityCRDB {

TEST(DeepFeatureTests, BTreeIndexSupportsRangeLookup) {
    BTreeIndex index;
    index.insert(Value{1}, 10);
    index.insert(Value{2}, 20);
    index.insert(Value{3}, 30);

    EXPECT_EQ(index.find(Value{2}), std::vector<RowId>{20});
    EXPECT_EQ(index.lessThan(Value{3}), (std::vector<RowId>{10, 20}));
    EXPECT_EQ(index.greaterThan(Value{1}), (std::vector<RowId>{20, 30}));
}

TEST(DeepFeatureTests, BufferPoolEvictsLeastRecentlyUsedPage) {
    BufferPool pool{2};
    pool.put(Page{1, {std::byte{1}}, false});
    pool.put(Page{2, {std::byte{2}}, false});
    ASSERT_TRUE(pool.get(1).has_value());
    pool.put(Page{3, {std::byte{3}}, false});

    EXPECT_TRUE(pool.contains(1));
    EXPECT_FALSE(pool.contains(2));
    EXPECT_TRUE(pool.contains(3));
}

TEST(DeepFeatureTests, WriteAheadLogAppendsAndReadsRecords) {
    const auto root = std::filesystem::temp_directory_path() /
                      ("theCityCRDB_wal_test_" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto path = root / "test.wal";

    WriteAheadLog wal{path};
    wal.reset();
    EXPECT_EQ(wal.append(WalOperation::CreateDatabase, "company"), 1U);
    EXPECT_EQ(wal.append(WalOperation::Insert, "Employees"), 2U);

    const auto records = wal.readAll();
    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].operation, WalOperation::CreateDatabase);
    EXPECT_EQ(records[0].payload, "company");
    EXPECT_EQ(records[1].operation, WalOperation::Insert);

    std::filesystem::remove_all(root);
}

TEST(DeepFeatureTests, PlannerChoosesIndexAccessPaths) {
    Table table{"Employees", {{"id", ColumnType::Int}, {"salary", ColumnType::Double}}};
    ASSERT_TRUE(table.createIndex("idx_id", "id"));
    ASSERT_TRUE(table.createIndex("idx_salary", "salary"));

    QueryPlanner planner;
    Select equality{"Employees", {"*"}, Predicate{"id", ComparisonOperator::Equal, Value{1}}, {}, {}};
    EXPECT_EQ(planner.planSelect(equality, table).accessPath, AccessPath::HashIndexLookup);

    Select range{"Employees", {"*"}, Predicate{"salary", ComparisonOperator::Greater, Value{100000.0}}, {}, {}};
    EXPECT_EQ(planner.planSelect(range, table).accessPath, AccessPath::OrderedIndexRange);
}

TEST(DeepFeatureTests, MVCCTracksTransactionVisibility) {
    TransactionManager transactions;
    const auto writer = transactions.begin();
    const auto reader = transactions.begin();

    MVCCRowStore store;
    store.write(1, {Value{10}}, writer.id);
    EXPECT_TRUE(store.read(1, reader.id).has_value());

    store.erase(1, reader.id);
    EXPECT_FALSE(store.read(1, reader.id).has_value());
    EXPECT_EQ(store.versionCount(1), 1U);
}

}  // namespace theCityCRDB
