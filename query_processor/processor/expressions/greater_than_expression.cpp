#include "greater_than_expression.hpp"

ExpressionVariant GreaterThanExpression::Eval(const Row& row) const
{
    ExpressionVariant leftOp = leftExpression->Eval(row);
    ExpressionVariant rightOp = rightExpression->Eval(row);

    if( ! (std::holds_alternative<int64_t>(leftOp) && std::holds_alternative<int64_t>(rightOp)) && !(std::holds_alternative<std::string>(leftOp) && std::holds_alternative<std::string>(rightOp)) )
    {
        throw std::invalid_argument("[Greater Than Expression - Eval]: Both operands should be of the same type");
    }

    if(std::holds_alternative<int64_t>(leftOp))
    {
        int64_t leftVal = std::get<int64_t>(leftOp);
        int64_t rightVal = std::get<int64_t>(rightOp);
        return leftVal > rightVal;
    }
    else if(std::holds_alternative<std::string>(leftOp))
    {
        std::string leftVal = std::get<std::string >(leftOp);
        std::string rightVal = std::get<std::string >(rightOp);
        
        return leftVal > rightVal;
    }

    return false;
}