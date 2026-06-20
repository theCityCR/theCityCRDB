# Build Plan

## Engineering Approach

AtlasDB should be developed as a sequence of small, testable slices. Each phase should leave the project compiling, with focused tests and a short design note explaining tradeoffs.

## Milestones

1. Repository foundation: CMake, CI, formatting, static analysis, module boundaries, and starter docs.
2. Storage engine: typed columns, row validation, table metadata, row identifiers, deletion strategy, and memory-layout measurements.
3. Parser: tokenizer, AST, grammar tests, error reporting, and SQL compatibility notes.
4. Query execution: projection, filtering, update, delete, ordering, limit, and result formatting.
5. Hash indexes: index metadata, maintenance on insert/update/delete, and indexed point lookup planning.
6. Persistence: binary format, version header, table schemas, row pages, round-trip tests, and corruption handling.
7. B+ tree indexes: page-oriented node layout, range queries, split/merge logic, and invariants tests.
8. Concurrency: reader/writer locking, lock ordering, concurrent query tests, and contention benchmarks.
9. Transactions: transaction state, rollback log, commit protocol, and isolation guarantees.
10. Optimization and reporting: benchmarks, profiling notes, graphs, and README interview narrative.

## Definition of Done

Each feature should include:

- Public interface and implementation.
- Unit tests covering expected behavior and edge cases.
- Error-handling behavior documented in `docs/sql.md` or module docs.
- Benchmark update when the feature affects performance-sensitive paths.
- Sanitizer-clean test run before merging.

## Key Tradeoffs

- Start with row-oriented storage because it is simpler to validate and easier to explain. Revisit columnar or page-based row storage after query execution stabilizes.
- Use table-level `std::shared_mutex` first. Fine-grained row/page locks should wait until concurrency tests show the bottleneck.
- Use a simple AST and direct executor before adding a cost-based planner. That keeps correctness testable before optimization work begins.
- Persist a versioned binary format from the beginning, even if the first format only stores metadata, so compatibility thinking is not bolted on later.
