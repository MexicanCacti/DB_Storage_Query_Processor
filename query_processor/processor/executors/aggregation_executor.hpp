#pragma once
#include "executor.hpp"
#include "expression.hpp"

#include <memory>
#include <algorithm>
#include <map>

enum class AggregationType
{
    COUNT = 0,
    MIN = 1,
    MAX = 2,
    SUM = 3
};

struct AggregateState
{
    std::int64_t count = 0;
    std::int64_t sum = 0;
    std::optional<FieldVariant> minVal;
    std::optional<FieldVariant> maxVal;
};

struct AggregationExpression
{
    Field outputField;
    AggregationType type;
    std::unique_ptr<Expression> expression;
};

struct GroupByExpression
{
    std::unique_ptr<Expression> expression;
    Field outputField;
};

using GroupingKeys = std::vector<FieldVariant>;

class AggregationExecutor : public Executor
{
    private:
        std::unique_ptr<Executor> childExecutor;
        std::vector<AggregationExpression> aggregates;
        std::vector<GroupByExpression> groupByExpressions;
        std::map<GroupingKeys, std::vector<AggregateState>> groups;
        QuerySchema outputSchema;
        std::vector<Row> resultRows;
        std::size_t currentRow = 0;
        bool hasAggregated = false;

    public:
        AggregationExecutor(std::unique_ptr<Executor> child, std::vector<GroupByExpression> groupExp, std::vector<AggregationExpression> aggs);
        void Init() override {childExecutor->Init(); groups.clear(); resultRows.clear(); currentRow = 0; hasAggregated = false;}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close(); groups.clear(); resultRows.clear(); currentRow = 0; hasAggregated = true;}
        const QuerySchema& OutputSchema() const override {return outputSchema;}
};