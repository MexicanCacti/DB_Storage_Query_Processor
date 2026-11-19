#pragma once
#include "executor.hpp"
#include "expression.hpp"
#include <memory>

class FilterExecutor : public Executor
{
    private:
        std::unique_ptr<Executor> childExecutor;
        std::unique_ptr<Expression> predicate;

    public:
        FilterExecutor(std::unique_ptr<Executor> child, std::unique_ptr<Expression> pred) : 
            childExecutor(std::move(child)), predicate(std::move(pred))
        {}
        void Init() override {childExecutor->Init();}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close();}
        const Schema& OutputSchema() const override {return childExecutor->OutputSchema();}
};