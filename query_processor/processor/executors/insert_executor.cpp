#include "insert_executor.hpp"

bool InsertExecutor::Next(Row& out)
{
    if(hasInserted) return false;

    std::vector<Row> insertRows;
    Row row;

    while(childExecutor->Next(row))
    {
        insertRows.push_back(row);
    }

    std::int64_t rows_inserted = 0;
    for(const Row& insertRow : insertRows)
    {
        RID insertRID = databasePointer->insertTuple(tableName, insertRow.tuple);
        if(insertRID.tupleSlot == -1)
        {
            out = Row{Tuple({rows_inserted}), std::nullopt};
            throw std::invalid_argument("[Insert Executor - Next]: Insert failed. Table full");
        }
        if(insertRID.pageID.tableID == -1)
        {
            out = Row{Tuple({rows_inserted}), std::nullopt};
            throw std::invalid_argument("[Insert Executor - Next]: Insert failed. Table does not exist");
        }
        ++rows_inserted;
    }
    
    hasInserted = true;
    
    out = Row{Tuple({rows_inserted}), std::nullopt};
    return true;
}