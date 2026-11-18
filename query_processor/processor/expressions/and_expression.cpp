#include "and_expression.hpp"

ExpressionVariant AndExpression::Eval(const Row& row) const
{
    ExpressionVariant leftOp = leftExpression->Eval(row);

    if(!std::holds_alternative<bool>(leftOp))
    {
        throw std::invalid_argument("[AND Expression - Eval]: Both operands should be Bool");
    }

    bool leftVal = std::get<bool>(leftOp);
    if(!leftVal) return false;

    ExpressionVariant rightOp = rightExpression->Eval(row);

    if(!std::holds_alternative<bool>(rightOp))
    {
        throw std::invalid_argument("[AND Expression - Eval]: Both operands should be Bool");
    }

    bool rightVal = std::get<bool>(rightOp);
    if(!rightVal) return false;

    return true;
}