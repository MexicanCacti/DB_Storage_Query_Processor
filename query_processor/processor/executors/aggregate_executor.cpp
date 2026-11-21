#include "aggregate_executor.hpp"


AggregationExecutor::AggregationExecutor(std::unique_ptr<Executor> child, std::vector<AggregationExpression> aggs)
{

}

void AggregationExecutor::Init()
{

}

bool AggregationExecutor::Next(Row& out)
{
    if(hasAggregated) return false;

    
}

void AggregationExecutor::Close()
{

}