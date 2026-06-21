# SQL Subset

theCityCRDB intentionally supports a small SQL subset first.

## Implemented

```sql
CREATE DATABASE company;
CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);
DROP TABLE Employees;
RENAME TABLE Employees TO Staff;
LIST TABLES;
INSERT INTO Employees VALUES (1, "Alice", 120000.0);
SELECT * FROM Employees;
SELECT name FROM Employees WHERE salary > 100000.0 LIMIT 10;
SELECT name FROM Employees WHERE salary > 100000.0 ORDER BY salary DESC LIMIT 10;
SELECT * FROM Employees JOIN Departments ON dept_id = id LIMIT 10;
SELECT Employees.name, Departments.dept
FROM Employees JOIN Departments ON Employees.dept_id = Departments.id
WHERE Departments.dept > "A"
ORDER BY Employees.name DESC
LIMIT 5;
UPDATE Employees SET salary = 150000.0 WHERE id = 1;
DELETE FROM Employees WHERE id = 5;
CREATE INDEX idx_salary ON Employees(salary);
PREPARE by_id AS "SELECT name FROM Employees WHERE id = ?;";
EXECUTE by_id VALUES (1);
SAVE DATABASE;
LOAD DATABASE;
LOAD DATABASE company;
BEGIN;
COMMIT;
ROLLBACK;
EXIT;
```

`CREATE INDEX` builds a hash index maintained by table writes. Equality predicates in `SELECT`
use the index when the filtered column is indexed.

`JOIN` supports a single equi-join. Joined result columns are qualified as `LeftTable.column` and
`RightTable.column`. Projection, `WHERE`, `ORDER BY`, and `LIMIT` can reference qualified columns;
unqualified references are allowed when the column name is not ambiguous.

Prepared statements store a SQL string containing `?` placeholders. `EXECUTE name VALUES (...)`
binds values positionally, reparses the bound statement, and executes it through the normal engine.

`SAVE DATABASE` and `LOAD DATABASE` use a versioned binary format under the executor's storage
root. `LOAD DATABASE` without a name reloads the active database when one exists, otherwise it
loads the first saved database file. Query executors also recover automatically on startup by
loading the latest saved snapshot and replaying WAL records after that checkpoint. If no snapshot
exists, startup recovery replays the WAL from the beginning.

Transactions use snapshot rollback semantics. `BEGIN` captures the active database state,
`COMMIT` releases the snapshot, and `ROLLBACK` restores it.

## Types

- `INT`: stored as signed 64-bit integer.
- `DOUBLE`: stored as C++ `double`.
- `STRING`: stored as `std::string`.

## Near-Term Grammar Work

- Optional semicolon enforcement.
- Better diagnostics with source positions.
- Multiple-row `INSERT`.
- `AND`/`OR` predicates.
