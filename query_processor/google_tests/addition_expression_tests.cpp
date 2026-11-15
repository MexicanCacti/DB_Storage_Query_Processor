#include <gtest/gtest.h>
#include <memory>

#include "expression.hpp"
#include "constant_expression.hpp"
#include "column_expression.hpp"
#include "addition_expression.hpp"

Tuple testTuple({
    int64_t{0},
    std::string{"A"},
    int64_t{-1},
    std::string{""}
});

TEST(AdditionExpressionTest, ColumnPlusConstant) {
    AdditionExpression expression(
        std::make_unique<ColumnExpression>(2),
        std::make_unique<ConstantExpression>(int64_t{5})
    );

    ExpressionVariant result =
        expression.Eval(Row{testTuple});

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));

    EXPECT_EQ(std::get<int64_t>(result), 4);
}