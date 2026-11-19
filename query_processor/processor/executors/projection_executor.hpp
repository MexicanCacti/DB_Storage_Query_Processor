#pragma once
#include "executor.hpp"
#include "expression.hpp"
#include "types.hpp"
#include <memory>
#include <vector>

struct ProjectionColumn
{
    Field projectionField;
    std::unique_ptr<Expression> projectionExpression;
};

class ProjectionExecutor : public Executor
{
    private:
        std::unique_ptr<Executor> childExecutor;
        std::vector<ProjectionColumn> projectionList;
        Schema projectionSchema;

    public:
        ProjectionExecutor(std::unique_ptr<Executor> child, std::vector<ProjectionColumn> pList);
        void Init() override {childExecutor->Init();}
        bool Next(Row& out) override;
        void Close() override {childExecutor->Close();}
        const Schema& OutputSchema() const override {return projectionSchema;}
};