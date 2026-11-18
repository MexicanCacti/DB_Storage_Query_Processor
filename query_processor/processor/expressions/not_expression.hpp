#pragma once

#include "expression.hpp"
#include <memory>

class NotExpression : public Expression 
{
    private:
        std::unique_ptr<Expression> expression;
    public:
        NotExpression(std::unique_ptr<Expression> expression) :
            expression(std::move(expression))
            {}

        ExpressionVariant Eval(const Row& row) const override;
};