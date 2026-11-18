#include "values_executor.hpp"

bool ValuesExecutor::Next(Row& out)
{
    if(currentRowIndex >= rowsPointer->size()) return false;

    out = (*rowsPointer)[currentRowIndex++];
    return true;
}