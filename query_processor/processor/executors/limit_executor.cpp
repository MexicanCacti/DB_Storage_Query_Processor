#include "limit_executor.hpp"

bool LimitExecutor::Next(Row& out)
{
    if(rowsGiven >= kVal || !childExecutor->Next(out)) return false;
    
    ++rowsGiven;

    return true;
}