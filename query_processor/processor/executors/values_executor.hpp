#pragma once
#include "executor.hpp"
#include <memory>

class ValuesExecutor : public Executor
{
    private:
        std::shared_ptr<const std::vector<Row>> rowsPointer;
        std::shared_ptr<const QuerySchema> schemaPointer;
        std::size_t currentRowIndex = 0;

    public:
        ValuesExecutor(std::shared_ptr<const std::vector<Row>> rows, std::shared_ptr<const QuerySchema> schema) :
            rowsPointer(std::move(rows)) , schemaPointer(schema)
            {}
        void Init() override {currentRowIndex = 0;}
        bool Next(Row& out) override;
        void Close() override {currentRowIndex = rowsPointer->size();}
        const QuerySchema& OutputSchema() const override {return *schemaPointer;}
};