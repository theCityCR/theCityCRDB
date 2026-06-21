# Architecture

```text
CLI
 |
 SQL Parser
 |
 Query Executor
 |
 +-- Storage Engine
 |   +-- Database
 |   +-- Table
 |   +-- RowStore
 |   |   +-- VectorRowStore
 |   |   +-- PageRowStore
 |   +-- Row
 |   +-- BufferPool
 |
 +-- Index Manager
 |   +-- HashIndex
 |   +-- BTreeIndex
 |       +-- BTreeNode layout metadata
 |
 +-- Persistence Layer
 |   +-- StorageManager
 |
 +-- Concurrency
     +-- LockManager
     +-- TransactionManager
     +-- MVCCRowStore
```

## Module Responsibilities

- `common`: shared value types, column metadata, and low-level utilities.
- `parser`: tokenization, AST construction, and SQL grammar validation.
- `storage`: database/table ownership, row storage boundaries, schema validation, and page cache
  abstractions.
- `execution`: command dispatch, predicate evaluation, projection, and result construction.
- `indexing`: hash indexes and ordered B+ tree index APIs with explicit node/page layout metadata.
- `persistence`: binary serialization, versioning, save/load, and recovery.
- `concurrency`: reader/writer synchronization and transaction coordination.

## Architectural Boundaries

`Table` owns schema validation and index maintenance, but delegates physical row storage to the
`RowStore` interface. `PageRowStore` is the default implementation and stores rows in compact
buffer-pool-backed pages. `VectorRowStore` remains available as a simple in-memory implementation
for focused tests or future comparisons.

`BTreeIndex` keeps the existing ordered lookup API while maintaining `BTreeNode` layout metadata
with page ids, leaf links, root children, separator keys, and row-id payloads in leaves. Lookup and
range reads use the leaf payloads; ordered entries remain as the mutation staging structure that
keeps node rebuilds deterministic.

`Table` exposes transaction-aware snapshots through `rowsSnapshot(TransactionId)` and
`rowsById(..., TransactionId)`. The executor routes active-transaction reads through those MVCC
APIs while retaining snapshot-copy rollback semantics for transaction undo.

## Current Data Flow

1. The CLI reads a SQL string.
2. `Tokenizer` emits a token stream.
3. `Parser` creates a strongly typed `Query` variant.
4. `QueryExecutor` dispatches the variant to storage operations.
5. Results are returned as `QueryResult` with columns, rows, and a status message.
