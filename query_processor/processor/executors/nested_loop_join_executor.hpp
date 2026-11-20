#pragma once
#include "executor.hpp"
#include "expression.hpp"

#include <memory>

class NestedLoopExecutor : public Executor
{
    private:
        std::unique_ptr<Executor> leftChildExecutor;
        std::unique_ptr<Executor> rightChildExecutor;
        std::unique_ptr<Expression> joinPredicate;
        std::shared_ptr<const QuerySchema> schemaPointer;
        std::vector<Row> rightChildRows;
        Row leftRow;
        std::size_t rightRowIndex = 0;

    public:
        NestedLoopExecutor(std::unique_ptr<Executor> leftChild, std::unique_ptr<Executor> rightChild, std::unique_ptr<Expression> pred);
        void Init() override;
        bool Next(Row& out) override;
        void Close() override {leftChildExecutor->Close(); rightChildExecutor->Close(); rightRowIndex = 0; rightChildRows.clear(); leftRow.tuple = Tuple({});}
        const QuerySchema& OutputSchema() const override {return *schemaPointer;}
};