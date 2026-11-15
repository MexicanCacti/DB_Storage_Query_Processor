#pragma once
#include "expression.hpp"

class ColumnExpression : public Expression
{
    private:
        std::size_t columnIndex;

    public:
        ColumnExpression(std::size_t index) :
            columnIndex(index)
        {}

        ExpressionVariant Eval(const Row& row) const override;
};