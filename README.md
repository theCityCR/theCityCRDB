# theCityCRDB

theCityCRDB is a C++20 in-memory relational database engine built as a systems programming portfolio project. The goal is not to compete with production databases; the goal is to show credible engineering decisions around storage, parsing, indexing, persistence, concurrency, testing, and performance.

## Current Status

This repository contains the initial engineering scaffold:

- CMake-based library and CLI targets.
- Typed row/table/database storage primitives.
- SQL tokenizer, parser, and AST for the first supported SQL subset.
- Execution engine for database/table creation, table listing, table rename/drop, insert, select,
  update, delete, index creation, save/load, and snapshot transactions.
- Maintained hash indexes with indexed equality lookup.
- Versioned binary persistence for schemas, rows, and index definitions.
- GoogleTest and Google Benchmark integration points.
- CI and quality tooling configuration.

## Build

```sh
cmake -S . -B build -DTHECITYCRDB_BUILD_TESTS=OFF
cmake --build build
```

Tests use GoogleTest through CMake FetchContent:

```sh
cmake -S . -B build -DTHECITYCRDB_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Example

```sql
CREATE DATABASE company;
CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);
INSERT INTO Employees VALUES (1, "Alice", 120000.0);
SELECT name FROM Employees WHERE salary > 100000.0 LIMIT 10;
```

## Roadmap

The detailed plan is in [docs/design.md](docs/design.md), with architecture notes in [docs/architecture.md](docs/architecture.md), SQL coverage in [docs/sql.md](docs/sql.md), and benchmark planning in [docs/benchmarks.md](docs/benchmarks.md).
