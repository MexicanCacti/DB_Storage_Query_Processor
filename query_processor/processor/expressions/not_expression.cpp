#include "not_expression.hpp"

ExpressionVariant NotExpression::Eval(const Row& row) const
{
    ExpressionVariant op = expression->Eval(row);

    if(!std::holds_alternative<bool>(op))
    {
        throw std::invalid_argument("[Not Expression - Eval]: Operand should be Bool");
    }

    bool val = std::get<bool>(op);

    return !val;
}