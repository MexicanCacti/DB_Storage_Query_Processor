#include "delete_executor.hpp"

bool DeleteExecutor::Next(Row& out)
{
    if(hasDeleted) return false;

    std::vector<Row> deleteRows;
    Row row;
    std::unordered_set<std::int16_t> seenRIDS;
    while(childExecutor->Next(row))
    {
        if(row.source.value().tableName != tableName || seenRIDS.find(row.source.value().rid.tupleSlot) != seenRIDS.end())
        {
            throw std::invalid_argument("[Delete Executor - Next]: Found row that is not from same table / has repeat RID");
        }
        deleteRows.push_back(row);
        seenRIDS.insert(row.source.value().rid.tupleSlot);
    }

    std::int64_t rows_deleted = 0;
    for(const Row& deleteRow : deleteRows)
    {
        databasePointer->deleteTuple(tableName, deleteRow.source.value().rid);
        ++rows_deleted;
    }
    
    hasDeleted = true;
    
    out = Row{Tuple({rows_deleted}), std::nullopt};
    return true;
}