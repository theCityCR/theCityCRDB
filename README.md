# VertexDB

VertexDB is a C++20 in-memory relational database engine built as a systems programming portfolio
project. It implements a practical SQL execution pipeline with typed storage, indexes, persistence,
write-ahead logging, concurrency control, transaction state, MVCC read paths, tests, benchmarks, and
multi-platform CI.

The project is intentionally scoped as a learning and interview project rather than a production
database. The emphasis is on clear architecture, modern C++ design, correctness tests, and the
tradeoffs behind database internals.

## Resume Highlights

- Built a relational database engine in modern C++20 with RAII, smart pointers, `std::variant`,
  `std::optional`, `std::span`, shared mutexes, and move-aware storage paths.
- Implemented SQL tokenization, parsing, AST construction, query planning, and execution for a
  focused SQL subset.
- Designed typed table storage with schema validation, nullable columns, page-backed row storage,
  and an LRU buffer pool abstraction.
- Implemented maintained hash indexes and ordered B+ tree-style range lookup APIs with explicit
  page/node metadata.
- Added versioned binary persistence, logical WAL records, save checkpoints, and startup recovery.
- Added transaction state tracking, MVCC row-version storage, transaction-aware reads, and
  snapshot rollback semantics.
- Built regression tests for WAL correctness, invalid mutation atomicity, nullable indexes,
  prepared statement escaping, joins, and concurrent execution.
- Configured GoogleTest, Google Benchmark, sanitizer builds, coverage checks, clang-tidy support,
  and GitHub Actions across GCC, Clang, macOS Clang, and MSVC.

## Architecture

```text
CLI
 |
 SQL Parser
 |
 Query Executor
 |
 +-- Query Planner
 +-- Storage Engine
 |   +-- Database / Table
 |   +-- RowStore
 |   |   +-- PageRowStore
 |   |   +-- VectorRowStore
 |   +-- BufferPool
 |
 +-- Index Manager
 |   +-- HashIndex
 |   +-- BTreeIndex
 |
 +-- Persistence
 |   +-- StorageManager
 |   +-- WriteAheadLog
 |
 +-- Concurrency / Transactions
     +-- LockManager
     +-- TransactionManager
     +-- MVCCRowStore
```

## Implemented SQL Surface

```sql
CREATE DATABASE company;
CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);
CREATE INDEX idx_salary ON Employees(salary);
INSERT INTO Employees VALUES (1, "Alice", 120000.0), (2, "Bob", 90000.0);
SELECT name FROM Employees WHERE salary > 100000.0 ORDER BY salary DESC LIMIT 10;
UPDATE Employees SET salary = 150000.0 WHERE id = 1;
DELETE FROM Employees WHERE id = 2;
SAVE DATABASE;
LOAD DATABASE company;
BEGIN;
COMMIT;
ROLLBACK;
```

Additional supported features include nullable columns, compound predicates, single equi-joins,
prepared statements with positional parameters, table rename/drop/list operations, and `EXIT`.

## Build And Test

```sh
cmake -S . -B build -DVERTEXDB_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Additional verification:

```sh
scripts/run-sanitizers.sh
scripts/run-coverage.sh
cmake -S . -B build-benchmark -DVERTEXDB_BUILD_TESTS=OFF -DVERTEXDB_BUILD_BENCHMARKS=ON
cmake --build build-benchmark
```

Run the CLI:

```sh
./build/VertexDB_cli
```

## Quality Bar

- 50 GoogleTest cases covering parser, storage, indexes, execution, persistence, WAL recovery,
  concurrency, transaction helpers, and regressions.
- Coverage script enforces an 85% line coverage floor for the core library.
- Sanitizer script runs AddressSanitizer and UndefinedBehaviorSanitizer on supported platforms.
- Benchmarks cover inserts, indexed and non-indexed filtered selects, update/delete throughput, and
  concurrent indexed point lookups.

## Current Limitations

- Row IDs are positional; deletes compact storage and can shift later row IDs.
- `PageRowStore` mirrors typed rows into buffer-pool pages, but typed row storage is still the
  operational source of truth.
- B+ tree writes rebuild a deterministic shallow layout instead of performing incremental
  split/merge operations.
- Transactions expose MVCC read boundaries, but rollback still restores a database snapshot copy.
- WAL recovery replays logical SQL operations rather than physical page redo records.
- The planner is rule-based and does not yet collect table/index statistics for costing.
- SQL support is intentionally focused: no aggregates, grouping, subqueries, or general DDL.

## Next Engineering Steps

1. Stabilize row IDs with tombstones and free-list reuse.
2. Make page storage the source of truth by deserializing rows from buffer-pool pages.
3. Replace snapshot-copy rollback with undo records or commit-aware MVCC visibility.
4. Implement incremental B+ tree split/merge with structural invariant tests.
5. Add physical WAL redo/recovery tests, including simulated partial writes.
6. Add table/index statistics and cost-based planning.
7. Extend SQL with aggregates, `COUNT`, `GROUP BY`, and broader join support.

See [docs/architecture.md](docs/architecture.md), [docs/design.md](docs/design.md),
[docs/sql.md](docs/sql.md), [docs/testing.md](docs/testing.md),
[docs/benchmarks.md](docs/benchmarks.md), and [docs/deep_features.md](docs/deep_features.md) for
more detail.
