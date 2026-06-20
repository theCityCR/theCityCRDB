#include "theCityCRDB/storage/table.hpp"

#include <benchmark/benchmark.h>

namespace theCityCRDB {
namespace {

void BM_InsertRows(benchmark::State& state) {
    for (auto _ : state) {
        Table table{"Employees", {{"id", ColumnType::Int}, {"name", ColumnType::String}}};
        for (std::int64_t i = 0; i < state.range(0); ++i) {
            table.insert({Value{i}, Value{std::string{"employee"}}});
        }
        benchmark::DoNotOptimize(table.rowCount());
    }
}

BENCHMARK(BM_InsertRows)->Arg(1000)->Arg(100000);

}  // namespace
}  // namespace theCityCRDB
