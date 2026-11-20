#pragma once
#include "executor.hpp"
#include "expression.hpp"

#include <memory>
#include <algorithm>

enum class SortDirection
{
    ASC = 0,
    DESC = 1
};

struct SortKey
{
    std::unique_ptr<Expression> sortExpression;
    SortDirection sortDirection;

};

struct SortByKeys {
    const std::vector<SortKey>& sortKeys;
    SortByKeys(const std::vector<SortKey>& keys) :
        sortKeys(keys)
    {}

    bool operator()(const Row& r1, const Row& r2) const {

        for(const SortKey& key : sortKeys)
        {
            ExpressionVariant r1Result = key.sortExpression->Eval(r1);
            if(std::holds_alternative<bool>(r1Result))
            {
                throw std::invalid_argument("[SortByKeys]: sortExpression should not return bool");
            }
            ExpressionVariant r2Result = key.sortExpression->Eval(r2);

            SortDirection keyDirection = key.sortDirection;
            if(r1Result > r2Result)
            {
                if(keyDirection == SortDirection::ASC) return false;
                else return true;
            }
            else if(r1Result < r2Result)
            {
                if(keyDirection == SortDirection::DESC) return false;
                else return true;
            }
        }

        return false;
    }
};

class SortExecutor : public Executor
{
    private:
        std::unique_ptr<Executor> childExecutor;
        std::vector<Row> sortedRows;
        std::size_t currentRowIndex = 0;
        std::vector<SortKey> sortKeys;

    public:
        SortExecutor(std::unique_ptr<Executor> child, std::vector<SortKey> keys) :
            childExecutor(std::move(child)), sortKeys(std::move(keys))
        {}
        void Init() override {childExecutor->Init(); sortedRows.clear(); currentRowIndex = 0;}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close(); sortedRows.clear(); currentRowIndex = 0;}
        const QuerySchema& OutputSchema() const override {return childExecutor->OutputSchema();}
};