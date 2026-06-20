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
 |   +-- Row
 |
 +-- Index Manager
 |   +-- HashIndex
 |   +-- BTreeIndex (planned)
 |
 +-- Persistence Layer
 |   +-- StorageManager
 |
 +-- Concurrency
     +-- LockManager
     +-- TransactionManager (planned)
```

## Module Responsibilities

- `common`: shared value types, column metadata, and low-level utilities.
- `parser`: tokenization, AST construction, and SQL grammar validation.
- `storage`: in-memory database, table, row, schema, and page abstractions.
- `execution`: command dispatch, predicate evaluation, projection, and result construction.
- `indexing`: hash indexes now; B+ tree indexes later.
- `persistence`: binary serialization, versioning, save/load, and recovery.
- `concurrency`: reader/writer synchronization and later transaction coordination.

## Current Data Flow

1. The CLI reads a SQL string.
2. `Tokenizer` emits a token stream.
3. `Parser` creates a strongly typed `Query` variant.
4. `QueryExecutor` dispatches the variant to storage operations.
5. Results are returned as `QueryResult` with columns, rows, and a status message.
