CREATE DATABASE company;
CREATE TABLE Employees (id INT, name STRING, salary DOUBLE);
INSERT INTO Employees VALUES (1, "Alice", 120000.0);
INSERT INTO Employees VALUES (2, "Bob", 90000.0);
SELECT name FROM Employees WHERE salary > 100000.0 LIMIT 10;
