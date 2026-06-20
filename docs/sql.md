# SQL Subset

AtlasDB intentionally supports a small SQL subset first.

## Implemented in the Initial Scaffold

```sql
CREATE DATABASE company;
CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);
INSERT INTO Employees VALUES (1, "Alice", 120000.0);
SELECT * FROM Employees;
SELECT name FROM Employees WHERE salary > 100000.0 LIMIT 10;
UPDATE Employees SET salary = 150000.0 WHERE id = 1;
DELETE FROM Employees WHERE id = 5;
CREATE INDEX idx_salary ON Employees(salary);
SAVE DATABASE;
LOAD DATABASE;
EXIT;
```

`CREATE INDEX`, `SAVE DATABASE`, and `LOAD DATABASE` are parsed and routed, but their complete behavior is planned for later milestones.

## Types

- `INT`: stored as signed 64-bit integer.
- `DOUBLE`: stored as C++ `double`.
- `STRING`: stored as `std::string`.

## Near-Term Grammar Work

- Optional semicolon enforcement.
- Better diagnostics with source positions.
- `ORDER BY`.
- Multiple-row `INSERT`.
- `AND`/`OR` predicates.
