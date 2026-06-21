# Deeper Feature Plan

## B+ Tree Index

The first implementation exposes B+ tree behavior through an ordered index API:

- point lookup
- less-than range lookup
- greater-than range lookup
- ordered row id storage per key

Internally it uses an ordered map-backed implementation behind the B+ tree API. This gives the
storage engine stable ordered-index behavior today while preserving the public boundary for a
future page-backed node implementation.

## Write-Ahead Log

The WAL records logical operations before mutating in-memory state:

- create database/table/index
- insert
- update
- delete
- drop/rename table
- save

The WAL persists append-only binary log records with a versioned header per record. Mutating
executor operations write replayable payloads. Startup recovery loads the latest saved snapshot,
then replays WAL records after the last save checkpoint. If no saved snapshot exists, recovery
replays the WAL from the beginning.

## Prepared Statements

Prepared statements keep named SQL templates with `?` placeholders. Execution binds literal values,
parses the resolved SQL, and routes it through the same planner and executor paths as direct SQL.
This keeps the first implementation compact while establishing a public API that can later move to
typed parameterized AST nodes.

## Joins

Joins support a single equi-join with projected or `SELECT *` output, qualified output column names,
joined `WHERE`, `ORDER BY`, and `LIMIT`. Execution builds an in-memory lookup over the right side
and scans the left side. Later planner work can choose between nested-loop and hash join variants
using table statistics.

## MVCC

The MVCC layer introduces transaction identifiers, transaction state management, and row-version
chains. `Table` now records row versions during inserts, updates, deletes, and row replacement so
real storage mutations maintain version metadata. User-facing transactions still use snapshot
rollback semantics while the version-chain data is available for future visibility rules.

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
