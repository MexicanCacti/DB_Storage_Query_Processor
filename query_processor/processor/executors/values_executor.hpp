#pragma once
#include "executor.hpp"
#include <memory>

class ValuesExecutor : public Executor
{
    private:
        std::shared_ptr<const std::vector<Row>> rowsPointer;
        std::shared_ptr<const Schema> schemaPointer;
        std::size_t currentRowIndex = 0;

    public:
        ValuesExecutor(std::shared_ptr<const std::vector<Row>> rows, std::shared_ptr<const Schema> schema) :
            rowsPointer(rows) , schemaPointer(schema)
            {}
        void Init() override {currentRowIndex = 0;}
        bool Next(Row& out) override;
        void Close() override {currentRowIndex = rowsPointer->size();}
        const Schema& OutputSchema() const override {return *schemaPointer;}
};