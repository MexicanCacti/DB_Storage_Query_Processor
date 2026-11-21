#include "aggregation_executor.hpp"


AggregationExecutor::AggregationExecutor(std::unique_ptr<Executor> child, std::vector<GroupByExpression> groupExp, std::vector<AggregationExpression> aggs) :
    childExecutor(std::move(child)), groupByExpressions(std::move(groupExp)), aggregates(std::move(aggs))
{
    std::vector<Field> outputField;

    for(const GroupByExpression& expression : groupByExpressions)
    {
        outputField.push_back(expression.outputField);
    }

    for(const AggregationExpression& aggregate : aggregates)
    {
        outputField.push_back(aggregate.outputField);
    }

    outputSchema = QuerySchema(std::move(outputField));
}

bool AggregationExecutor::Next(Row& out)
{
    if(!hasAggregated)
    {
        Row childRow;
        while(childExecutor->Next(childRow))
        {
            GroupingKeys groupKeys;

            // Build Keys
            for(const GroupByExpression& groupExpression : groupByExpressions)
            {
                ExpressionVariant expressionResult = groupExpression.expression->Eval(childRow);
                if(std::holds_alternative<bool>(expressionResult))
                {
                    throw std::invalid_argument("[Aggregation Executor - Next]: GroupBy expression can't return bool");
                }

                if(std::holds_alternative<int64_t>(expressionResult))
                {
                    if(groupExpression.outputField.type != FieldType::INT64)
                    {
                        throw std::invalid_argument("[Aggregation Executor - Next]: Grouping type mismatch");
                    }
                    groupKeys.push_back(std::get<int64_t>(expressionResult));
                }
                if(std::holds_alternative<std::string>(expressionResult))
                {
                    if(groupExpression.outputField.type != FieldType::STRING32)
                    {
                        throw std::invalid_argument("[Aggregation Executor - Next]: Grouping type mismatch");
                    }
                    groupKeys.push_back(std::get<std::string>(expressionResult));
                }
            }
            // Search for group key, DNE: create new group & init then get reference to agg states
            auto it = groups.find(groupKeys);
            if(it == groups.end())
            {
                std::vector<AggregateState> aggregateStates(aggregates.size());
                auto result = groups.insert({std::move(groupKeys), std::move(aggregateStates)});
                it = result.first;
            }
            std::vector<AggregateState>& aggregateStates = it->second;

            //
            for(std::size_t i = 0 ; i < aggregates.size(); ++i)
            {
                const AggregationExpression& aggregate = aggregates[i];
                AggregateState& aggState = aggregateStates[i];
                ExpressionVariant var; 
                int64_t expressionVal;
                int64_t aggregateVal;
                switch(aggregate.type)
                {
                    case AggregationType::COUNT:
                        ++aggState.count;
                        break;
                    case AggregationType::SUM:
                        var = aggregate.expression->Eval(childRow);
                        if(!std::holds_alternative<int64_t>(var))
                        {
                            throw std::invalid_argument("[Aggregate Executor - Next]: Sum requires int64_t expression result");
                        }
                        expressionVal = std::get<int64_t>(var);
                        aggregateVal = aggState.sum;

                        // int overflow check

                        if( (aggregateVal > 0 && expressionVal > std::numeric_limits<int64_t>::max() - aggregateVal) || (aggregateVal < 0 && expressionVal < std::numeric_limits<int64_t>::min() - aggregateVal) )
                        {
                            throw std::overflow_error("[Aggregation Executor - Next]: Overflow");
                        }
                        aggState.sum += expressionVal;
                        break;
                    case AggregationType::MIN:
                        var = aggregate.expression->Eval(childRow);
                        if(!std::holds_alternative<int64_t>(var))
                        {
                            throw std::invalid_argument("[Aggregate Executor - Next]: Min requires int64_t expression result");
                        }
                        if(!aggState.minVal.has_value() || std::get<int64_t>(aggState.minVal.value()) > std::get<int64_t>(var)) aggState.minVal = std::get<int64_t>(var);
                        break;
                    case AggregationType::MAX:
                        var = aggregate.expression->Eval(childRow);
                        if(!std::holds_alternative<int64_t>(var))
                        {
                            throw std::invalid_argument("[Aggregate Executor - Next]: Max requires int64_t expression result");
                        }
                        if(!aggState.maxVal.has_value() || std::get<int64_t>(aggState.maxVal.value()) < std::get<int64_t>(var)) aggState.maxVal = std::get<int64_t>(var);
                        break;
                    default:
                        throw std::invalid_argument("[Aggregation Executor - Next]: Unknown aggregation type");
                }
            }
        }


        for(const auto& [groupKey, aggStates] : groups)
        {
            std::vector<FieldVariant> output = groupKey;
            
            for(std::size_t i = 0 ; i < aggregates.size(); ++i)
            {
                const AggregationExpression& aggregate = aggregates[i];
                const AggregateState& aggState = aggStates[i];

                switch(aggregate.type)
                {
                    case AggregationType::COUNT:
                        output.push_back(aggState.count);
                        break;
                    case AggregationType::SUM:
                        output.push_back(aggState.sum);
                        break;
                    case AggregationType::MIN:
                        if(!aggState.minVal.has_value())
                        {
                            throw std::invalid_argument("[Aggregation Executor - Next]: Min missing a value");
                        }
                        output.push_back(std::get<int64_t>(aggState.minVal.value()));
                        break;
                    case AggregationType::MAX:
                        if(!aggState.maxVal.has_value())
                        {
                            throw std::invalid_argument("[Aggregation Executor - Next]: Max missing a value");
                        }
                        output.push_back(std::get<int64_t>(aggState.maxVal.value()));
                        break;
                    default:
                        throw std::invalid_argument("[Aggregation Executor - Next]: Unknown aggregation type");
                }
            }
            resultRows.push_back(Row{Tuple(std::move(output)), std::nullopt});
        }

        hasAggregated = true;
    }

    
    if(currentRow >= resultRows.size()) return false;

    out = resultRows[currentRow++];
    return true;
}