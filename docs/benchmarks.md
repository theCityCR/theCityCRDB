# Benchmark Plan

Benchmarks should be added alongside the features they measure.

## Initial Metrics

- Insert throughput for 1,000, 100,000, and 1,000,000 rows.
- Full table scan latency.
- Predicate-filtered query latency.
- Indexed point lookup latency.
- Update and delete throughput.
- Concurrent read scaling.

The current benchmark target includes insert throughput, indexed point lookup, and filtered
indexed select. Additional update/delete/concurrency cases should be added as those paths are
optimized.

## Reporting

Benchmark output should be checked into documentation only as summarized tables or generated graphs, not raw build artifacts.

Suggested comparisons:

- Indexed vs. non-indexed lookup.
- Single-thread read vs. multi-thread read.
- Debug vs. release builds.
- Sanitized vs. unsanitized builds.
