#pragma once
#include "schema.hpp"
#include "tuple.hpp"
#include "data_page.hpp"

#include <stdexcept>
#include <optional>

using ExpressionVariant = std::variant<int64_t, std::string, bool>; 

struct SourceReference{
    std::string tableName;
    RID recordID;
};

struct Row {
    Tuple tuple;
    std::optional<SourceReference> source;
};
