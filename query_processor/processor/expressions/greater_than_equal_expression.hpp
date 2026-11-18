#pragma once

#include "expression.hpp"
#include <memory>

class GreaterThanEqualExpression : public Expression 
{
    private:
        std::unique_ptr<Expression> leftExpression;
        std::unique_ptr<Expression> rightExpression;
    public:
        GreaterThanEqualExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) :
            leftExpression(std::move(left)), rightExpression(std::move(right))
            {}

        ExpressionVariant Eval(const Row& row) const override;
};