#pragma once
#include "executor.hpp"
#include "expression.hpp"
#include <memory>

class LimitExecutor : public Executor
{
    private:
        std::unique_ptr<Executor> childExecutor;
        std::size_t kVal = 0;
        std::size_t rowsGiven = 0;

    public:
        LimitExecutor(std::unique_ptr<Executor> child, std::size_t limit) : 
            childExecutor(std::move(child)), kVal(limit)
        {}
        void Init() override {childExecutor->Init(); rowsGiven = 0;}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close(); rowsGiven = kVal;}
        const Schema& OutputSchema() const override {return childExecutor->OutputSchema();}
};