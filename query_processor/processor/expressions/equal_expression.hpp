#pragma once

#include "expression.hpp"
#include <memory>

class EqualExpression : public Expression 
{
    private:
        std::unique_ptr<Expression> leftExpression;
        std::unique_ptr<Expression> rightExpression;
    public:
        EqualExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) :
            leftExpression(std::move(left)), rightExpression(std::move(right))
            {}

        ExpressionVariant Eval(const Row& row) const override;
};