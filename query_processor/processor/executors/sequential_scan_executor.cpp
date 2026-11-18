#include "sequential_scan_executor.hpp"

SequentialScanExecutor::SequentialScanExecutor(std::shared_ptr<Database> database, std::string tableName)
    : databasePointer(database), tableName(tableName)
{
    TableHeader tableInfo = databasePointer->queryTableInfo(this->tableName);
    schemaPointer = std::make_shared<const Schema>(tableInfo.getSchema());

}

void SequentialScanExecutor::Init()
{
    currentRowIndex = 0;
    std::vector<std::pair<Tuple, RID>> tableScanResults = databasePointer->scan(tableName);
    rows.clear();
    rows.reserve(tableScanResults.size());

    for(auto& result : tableScanResults)
    {
        rows.push_back({result.first, SourceReference{tableName, result.second}});
    }
}

bool SequentialScanExecutor::Next(Row& out)
{
    if(currentRowIndex >= rows.size()) return false;

    out = rows[currentRowIndex++];
    return true;
}