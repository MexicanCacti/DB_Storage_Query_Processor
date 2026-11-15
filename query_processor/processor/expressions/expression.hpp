#pragma once
#include "types.hpp"
#include <variant>

class Expression {
    private:

    public:
        virtual ~Expression() = default;
        virtual ExpressionVariant Eval(const Row& row) const = 0;

};