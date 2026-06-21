# Deeper Feature Plan

## B+ Tree Index

The first implementation exposes B+ tree behavior through an ordered index API:

- point lookup
- less-than range lookup
- greater-than range lookup
- ordered row id storage per key

Internally it starts with `std::map` so callers and tests can stabilize before replacing the
implementation with page-backed B+ tree nodes and split/merge logic.

## Write-Ahead Log

The WAL records logical operations before mutating in-memory state:

- create database/table/index
- insert
- update
- delete
- drop/rename table
- save

The first implementation persists append-only binary log records with a versioned header per
record. Replay is exposed as a low-level API. The executor currently performs automatic startup
recovery by loading the first saved database snapshot from disk. Full crash recovery will later map
WAL records back through the SQL parser/executor or a typed recovery executor.

## Prepared Statements

Prepared statements keep named SQL templates with `?` placeholders. Execution binds literal values,
parses the resolved SQL, and routes it through the same planner and executor paths as direct SQL.
This keeps the first implementation compact while establishing a public API that can later move to
typed parameterized AST nodes.

## Joins

The first join implementation supports a single equi-join with `SELECT *`, optional `LIMIT`, and
qualified output column names. It builds an in-memory lookup over the right side and scans the left
side. Later work should add projected join columns, predicate pushdown, and planner-selected join
algorithms such as nested-loop versus hash join.

## MVCC

The first MVCC layer introduces transaction identifiers and transaction state management. Storage
still uses snapshot rollback transactions today. A later storage-engine pass should move `Table`
from a single row vector to version chains with begin/end transaction ids.

## Buffer Pool

The buffer pool is an LRU cache for fixed-size page payloads. It is independent from table storage
for now, which lets persistence and future page-oriented B+ tree storage adopt it incrementally.

## Query Planner

The first planner chooses between:

- full table scan
- hash index equality lookup
- ordered index range lookup

This gives the executor an explicit planning step without introducing a cost model that is more
complicated than the available statistics justify.
