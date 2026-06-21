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
 |   |   +-- PageRowStore (next)
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
`RowStore` interface. `VectorRowStore` preserves current in-memory behavior. A future
`PageRowStore` can use `BufferPool` without changing executor or database code.

`BTreeIndex` keeps the existing ordered lookup API while maintaining `BTreeNode` layout metadata
with page ids, leaf links, root children, and separator keys. The current implementation still uses
ordered entries for lookup correctness, but the page metadata is the boundary for replacing it with
real split/merge node operations.

`Table` exposes transaction-aware snapshots through `rowsSnapshot(TransactionId)` and
`rowsById(..., TransactionId)`. Normal executor reads still use the latest committed physical view,
while future isolation work can route transactional reads through the MVCC boundary.

## Current Data Flow

1. The CLI reads a SQL string.
2. `Tokenizer` emits a token stream.
3. `Parser` creates a strongly typed `Query` variant.
4. `QueryExecutor` dispatches the variant to storage operations.
5. Results are returned as `QueryResult` with columns, rows, and a status message.
