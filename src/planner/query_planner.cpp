#include "theCityCRDB/planner/query_planner.hpp"

namespace theCityCRDB {

QueryPlan QueryPlanner::planSelect(const Select &query, const Table &table) const {
    QueryPlan plan;
    plan.predicate = query.where;
    if (!query.where) {
        return plan;
    }

    if (query.where->op == ComparisonOperator::Equal && table.hasIndex(query.where->column)) {
        plan.accessPath = AccessPath::HashIndexLookup;
        plan.explanation = "hash index equality lookup";
        return plan;
    }

    if ((query.where->op == ComparisonOperator::Greater ||
         query.where->op == ComparisonOperator::Less) &&
        table.hasOrderedIndex(query.where->column)) {
        plan.accessPath = AccessPath::OrderedIndexRange;
        plan.explanation = "ordered index range lookup";
        return plan;
    }

    return plan;
}

} // namespace theCityCRDB
