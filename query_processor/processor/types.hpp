#pragma once
#include "schema.hpp"
#include "tuple.hpp"

using ExpressionVariant = std::variant<int64_t, std::string, bool>; 

struct SourceReference{
    std::string tableName;
    RID recordID;
};

struct Row {
    Tuple tuple;
    SourceReference source;
};
