#pragma once
#include "schema.hpp" // Schema
#include "tuple.hpp"

struct SourceReference{
    std::string tableName;
    RID recordID;
};

struct Row {
    Tuple tuple;
    SourceReference source;
};


class Exectuor
{
    public:
        virtual ~Executor() = default;
        virtual void Init() = 0;
        virtual bool Next(Row& out) = 0;
        virtual Close() = 0;
        virtual const Schema& OutputSchema() const = 0;

};