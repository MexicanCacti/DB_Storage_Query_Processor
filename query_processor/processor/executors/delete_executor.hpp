#pragma once
#include "executor.hpp"
#include <memory>
#include <unordered_set>

class DeleteExecutor : public Executor
{
    private:
        std::shared_ptr<Database> databasePointer;
        std::string tableName;
        std::unique_ptr<Executor> childExecutor;
        QuerySchema affectedRows;
        bool hasDeleted = false;

    public:
        DeleteExecutor(std::shared_ptr<Database> database, std::string tableName, std::unique_ptr<Executor> child) :
            databasePointer(database), tableName(tableName), childExecutor(std::move(child)), affectedRows(std::vector<Field>{Field{"Affected Rows", FieldType::INT64}})
        {}
        void Init() override {childExecutor->Init(); hasDeleted = false;}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close(); hasDeleted = true;}
        const QuerySchema& OutputSchema() const override {return affectedRows;}
};