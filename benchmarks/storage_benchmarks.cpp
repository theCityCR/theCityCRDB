#include "theCityCRDB/storage/table.hpp"

#include "theCityCRDB/execution/query_executor.hpp"
#include "theCityCRDB/parser/parser.hpp"

#include <benchmark/benchmark.h>

namespace theCityCRDB {
namespace {

void BM_InsertRows(benchmark::State &state) {
    for (auto _ : state) {
        Table table{"Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}};
        for (std::int64_t i = 0; i < state.range(0); ++i) {
            table.insert({Value{i}, Value{std::string{"employee"}}});
        }
        benchmark::DoNotOptimize(table.rowCount());
    }
}

BENCHMARK(BM_InsertRows)->Arg(1000)->Arg(100000);

void BM_IndexedPointLookup(benchmark::State &state) {
    Table table{"Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}};
    for (std::int64_t i = 0; i < state.range(0); ++i) {
        table.insert({Value{i}, Value{std::string{"employee"}}});
    }
    table.createIndex("idx_id", "id");

    for (auto _ : state) {
        auto rows = table.findIndexed("id", Value{static_cast<std::int64_t>(state.range(0) / 2)});
        benchmark::DoNotOptimize(rows);
    }
}

BENCHMARK(BM_IndexedPointLookup)->Arg(1000)->Arg(100000);

void BM_FilteredSelect(benchmark::State &state) {
    Parser parser;
    QueryExecutor executor;
    (void)executor.execute(parser.parse("CREATE DATABASE bench;"));
    (void)executor.execute(parser.parse("CREATE TABLE Employees (id INT, name STRING);"));
    (void)executor.execute(parser.parse("CREATE INDEX idx_id ON Employees(id);"));
    for (std::int64_t i = 0; i < state.range(0); ++i) {
        (void)executor.execute(Insert{"Employees", {{Value{i}, Value{std::string{"employee"}}}}});
    }
    const auto query = parser.parse("SELECT name FROM Employees WHERE id = 500;");

    for (auto _ : state) {
        auto result = executor.execute(query);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_FilteredSelect)->Arg(1000)->Arg(100000);

} // namespace
} // namespace theCityCRDB
