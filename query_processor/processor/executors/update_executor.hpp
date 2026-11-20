#pragma once
#include "executor.hpp"
#include "expression.hpp"
#include <memory>
#include <unordered_set>

struct UpdateExpression
{
    std::size_t columnIndex;
    std::unique_ptr<Expression> updateExpression;
};

struct Update
{
    RID rid;
    Tuple updateTuple;
};

class UpdateExecutor : public Executor
{
    private:
        std::shared_ptr<Database> databasePointer;
        std::string tableName;
        std::unique_ptr<Executor> childExecutor;
        std::vector<UpdateExpression> updateExpressions;
        QuerySchema affectedRows;
        bool hasUpdated = false;

    public:
        UpdateExecutor(std::shared_ptr<Database> database, std::string tableName, std::unique_ptr<Executor> child, std::vector<UpdateExpression> expressions) :
            databasePointer(database), tableName(tableName), childExecutor(std::move(child)), 
            updateExpressions(std::move(expressions)), affectedRows(std::vector<Field>{Field{"Affected Rows", FieldType::INT64}})
        {}
        void Init() override {childExecutor->Init(); hasUpdated = false;}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close(); hasUpdated = true;}
        const QuerySchema& OutputSchema() const override {return affectedRows;}
};