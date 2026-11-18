#pragma once
#include "types.hpp"
#include <variant>
#include <limits>

class Expression {
    private:

    public:
        virtual ~Expression() = default;
        virtual ExpressionVariant Eval(const Row& row) const = 0;

};