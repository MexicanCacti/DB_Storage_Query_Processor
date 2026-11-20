#pragma once
#include "executor.hpp"
#include <memory>

class InsertExecutor : public Executor
{
    private:
        std::shared_ptr<Database> databasePointer;
        std::string tableName;
        std::unique_ptr<Executor> childExecutor;
        QuerySchema affectedRows;
        bool hasInserted = false;

    public:
        InsertExecutor(std::shared_ptr<Database> database, std::string tableName, std::unique_ptr<Executor> child) :
            databasePointer(database), tableName(tableName), childExecutor(std::move(child)), affectedRows(std::vector<Field>{Field{"Affected Rows", FieldType::INT64}})
        {}
        void Init() override {childExecutor->Init(); hasInserted = false;}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close(); hasInserted = true;}
        const QuerySchema& OutputSchema() const override {return affectedRows;}
};