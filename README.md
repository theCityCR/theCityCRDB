# theCityCRDB

theCityCRDB is a C++20 in-memory relational database engine built as a systems programming
portfolio project. It is not intended to compete with production databases. Its purpose is to show
credible engineering decisions around storage, parsing, indexing, persistence, concurrency,
transactions, testing, and performance.

## Current Features

- CMake-based library, CLI, test, sanitizer, coverage, and benchmark targets.
- SQL tokenizer, parser, and strongly typed AST for the supported SQL subset.
- Database and table management: create database, create/drop/rename/list tables.
- Typed rows with `INT`, `DOUBLE`, `STRING`, and nullable columns.
- Page-backed row storage through `PageRowStore`, with `VectorRowStore` retained for simple
  in-memory storage use cases.
- Buffer pool with LRU eviction for page payloads.
- Query execution for `INSERT`, `SELECT`, `UPDATE`, `DELETE`, `ORDER BY`, `LIMIT`, predicates, and
  a single equi-join.
- Hash indexes for equality lookup and ordered B+ tree index APIs for point/range lookup.
- B+ tree layout metadata with page ids, linked leaves, root separator keys, and row-id payloads in
  leaf nodes.
- Query planner that chooses between full scans, hash index equality lookups, and ordered index
  range lookups.
- Versioned binary persistence for schemas, rows, and index definitions.
- Logical write-ahead log with startup recovery and save checkpoints.
- Prepared statements with positional `?` parameters.
- Reader/writer synchronization for concurrent query execution.
- Transaction state tracking, MVCC row-version storage, transaction-aware reads, and
  snapshot-copy rollback semantics.
- GoogleTest coverage for parser, storage, indexes, execution, persistence, WAL recovery,
  transactions, concurrency, and regressions.
- Google Benchmark coverage for insert throughput, indexed and non-indexed lookups, update/delete
  throughput, and concurrent indexed reads.
- GitHub Actions CI for GCC, Clang, macOS Clang, and Windows MSVC.

## Build

```sh
cmake -S . -B build -DTHECITYCRDB_BUILD_TESTS=OFF
cmake --build build
```

Build and run tests:

```sh
cmake -S . -B build -DTHECITYCRDB_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the local verification suite:

```sh
scripts/run-sanitizers.sh
scripts/run-coverage.sh
cmake -S . -B build-benchmark -DTHECITYCRDB_BUILD_BENCHMARKS=ON
cmake --build build-benchmark
```

## Example

```sql
CREATE DATABASE company;
CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);
CREATE INDEX idx_salary ON Employees(salary);
INSERT INTO Employees VALUES (1, "Alice", 120000.0), (2, "Bob", 90000.0);
SELECT name FROM Employees WHERE salary > 100000.0 ORDER BY salary DESC LIMIT 10;
SAVE DATABASE;
```

## Current Limitations

- Row IDs are still positional; deletes compact storage and can shift later row IDs.
- `PageRowStore` mirrors typed rows into buffer-pool pages, but typed row storage is still the
  operational source of truth.
- B+ tree writes rebuild a deterministic shallow layout instead of performing incremental
  split/merge operations.
- Transactions expose MVCC read boundaries, but rollback still restores a database snapshot copy.
- WAL recovery replays logical SQL operations rather than physical page redo records.
- The planner is rule-based and does not yet collect table/index statistics for costing.
- SQL support is intentionally small: no aggregates, grouping, subqueries, multi-join planning, or
  general DDL.

## Next Planned Steps

1. Stabilize row IDs with tombstones and free-list reuse.
2. Make page storage the source of truth by deserializing rows from buffer-pool pages.
3. Replace snapshot-copy rollback with real undo/MVCC commit visibility.
4. Implement incremental B+ tree split/merge and structural invariant tests.
5. Add physical WAL redo/recovery tests, including simulated partial writes.
6. Add table/index statistics and a cost-based planner path.
7. Extend SQL with aggregates, `COUNT`, `GROUP BY`, and broader join support.
8. Publish benchmark summaries and trend graphs in `docs/benchmarks.md`.

More detail is available in [docs/architecture.md](docs/architecture.md),
[docs/design.md](docs/design.md), [docs/sql.md](docs/sql.md),
[docs/testing.md](docs/testing.md), [docs/benchmarks.md](docs/benchmarks.md), and
[docs/deep_features.md](docs/deep_features.md).
