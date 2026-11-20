#pragma once
#include "types.hpp"
#include "database.hpp"
#include <algorithm>

// Needed b/c storage's query does not allow duplicate schema field names
// V. Important for nested_loop_join
class QuerySchema
{
    private:
        std::vector<Field> querySchemaField;
    public:
        QuerySchema() = default;
        QuerySchema(std::vector<Field> fields) :
            querySchemaField(std::move(fields))
        {}
        QuerySchema(const Schema& schema) :
            querySchemaField(schema.getFieldList())
        {}

        const std::vector<Field>& getFieldList() const {return querySchemaField;}
};

class Executor
{
    private:

    public:
        virtual ~Executor() = default;
        virtual void Init() = 0;
        virtual bool Next(Row& out) = 0;
        virtual void Close() = 0;
        virtual const QuerySchema& OutputSchema() const = 0;

};