#include "theCityCRDB/planner/query_planner.hpp"

namespace theCityCRDB {
namespace {

const Predicate *simpleComparison(const std::optional<Predicate> &predicate) {
    if (!predicate || predicate->kind != Predicate::Kind::Comparison) {
        return nullptr;
    }
    return &*predicate;
}

} // namespace

QueryPlan QueryPlanner::planSelect(const Select &query, const Table &table) const {
    QueryPlan plan;
    plan.predicate = query.where;
    const auto *predicate = simpleComparison(query.where);
    if (predicate == nullptr) {
        return plan;
    }

    if (predicate->op == ComparisonOperator::Equal && table.hasIndex(predicate->column)) {
        plan.accessPath = AccessPath::HashIndexLookup;
        plan.explanation = "hash index equality lookup";
        return plan;
    }

    if ((predicate->op == ComparisonOperator::Greater ||
         predicate->op == ComparisonOperator::Less) &&
        table.hasOrderedIndex(predicate->column)) {
        plan.accessPath = AccessPath::OrderedIndexRange;
        plan.explanation = "ordered index range lookup";
        return plan;
    }

    return plan;
}

} // namespace theCityCRDB
