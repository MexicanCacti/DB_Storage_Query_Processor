#include "addition_expression.hpp"

ExpressionVariant AdditionExpression::Eval(const Row& row) const
{
    ExpressionVariant leftOp = left->Eval(row);
    ExpressionVariant rightOp = right->Eval(row);

    if(!std::holds_alternative<int64_t>(leftOp) || !std::holds_alternative<int64_t>(rightOp))
    {
        throw std::invalid_argument("[Addition Expression - Eval]: Both operands should be INT64");
    }

    int64_t leftVal = std::get<int64_t>(leftOp);
    int64_t rightVal = std::get<int64_t>(rightOp);

    // int overflow check

    return leftVal + rightVal;
}