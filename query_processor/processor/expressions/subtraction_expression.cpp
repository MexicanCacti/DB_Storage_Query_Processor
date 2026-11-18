#include "subtraction_expression.hpp"

ExpressionVariant SubtractionExpression::Eval(const Row& row) const
{
    ExpressionVariant leftOp = leftExpression->Eval(row);
    ExpressionVariant rightOp = rightExpression->Eval(row);

    if(!std::holds_alternative<int64_t>(leftOp) || !std::holds_alternative<int64_t>(rightOp))
    {
        throw std::invalid_argument("[Subtraction Expression - Eval]: Both operands should be INT64");
    }

    int64_t leftVal = std::get<int64_t>(leftOp);
    int64_t rightVal = std::get<int64_t>(rightOp);

    // int overflow check

    if( (rightVal < 0 && leftVal > std::numeric_limits<int64_t>::max() + rightVal) || (rightVal > 0 && leftVal < std::numeric_limits<int64_t>::min() + rightVal) )
    {
        throw std::overflow_error("[Subtraction Expression - Eval]: Overflow");
    }

    ExpressionVariant retVar = leftVal - rightVal;
    return retVar;
}