#include "column_expression.hpp"

ExpressionVariant ColumnExpression::Eval(const Row& row) const {

    std::vector<FieldVariant> tupleValues = row.tuple.getTupleValues();

    if(columnIndex >= tupleValues.size())
    {
        throw std::out_of_range("[ColumnExpression - Eval]: Index column invalid");
    }

    FieldVariant tupleVal = tupleValues[columnIndex];

    ExpressionVariant retVar = false;
    if(std::holds_alternative<int64_t>(tupleVal))
    {
        retVar = std::get<int64_t>(tupleVal);
    }
    else if(std::holds_alternative<std::string>(tupleVal))
    {
        retVar = std::get<std::string>(tupleVal);
    }

    return retVar;
}