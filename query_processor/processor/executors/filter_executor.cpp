#include "filter_executor.hpp"

bool FilterExecutor::Next(Row& out)
{
    while(childExecutor->Next(out))
    {
        ExpressionVariant childResult = predicate->Eval(out);

        if(!std::holds_alternative<bool>(childResult))
        {
            throw std::invalid_argument("[FilterExecutor - Next]: Filter predicate should return bool");
        }

        if(std::get<bool>(childResult))
        {
            return true;
        }
    }

    return false;
}