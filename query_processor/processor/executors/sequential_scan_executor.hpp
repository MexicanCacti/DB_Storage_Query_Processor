#pragma once
#include "executor.hpp"
#include <memory>

class SequentialScanExecutor : public Executor
{
    private:
        std::shared_ptr<Database> databasePointer;
        std::string tableName;
        std::vector<Row> rows;
        std::shared_ptr<const QuerySchema> schemaPointer;
        std::size_t currentRowIndex = 0;

    public:
        SequentialScanExecutor(std::shared_ptr<Database> database, std::string tableName); // Get QuerySchema : Database queryTableInfo(), 
        void Init() override; // call Database Scan(), use results to make std::vector<Row> & populate rows
        bool Next(Row& out) override;
        void Close() override {rows.clear(); currentRowIndex = 0;}
        const QuerySchema& OutputSchema() const override {return *schemaPointer;}
};