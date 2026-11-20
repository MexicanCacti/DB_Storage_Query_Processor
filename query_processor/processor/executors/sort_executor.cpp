#include "sort_executor.hpp"

bool SortExecutor::Next(Row& out)
{
    if(currentRowIndex == 0) // First call, populate sorted rows
    {
        Row childRow;
        while(childExecutor->Next(childRow))
        {
            sortedRows.push_back(std::move(childRow));
        }
        
        std::sort(sortedRows.begin(), sortedRows.end(), SortByKeys(sortKeys)); // Note: probably need default comparison for Row
    }
    if(currentRowIndex >= sortedRows.size()) return false;

    out = sortedRows[currentRowIndex++];
    return true;
}