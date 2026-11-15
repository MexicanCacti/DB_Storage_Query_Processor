#pragma once
#include "expression.hpp"

class ConstantExpression : public Expression 
{
    private:
        ExpressionVariant var;

    public:
        ConstantExpression(ExpressionVariant variant) : 
            var(std::move(variant)) 
        {}

        ExpressionVariant Eval(const Row& row) const override {return var;}
};