#include <gtest/gtest.h>
#include <memory>

#include "expression.hpp"
#include "constant_expression.hpp"
#include "column_expression.hpp"
#include "addition_expression.hpp"

TEST(AdditionExpressionTest, ColumnPlusConstant) {

    // Create a tuple representing a student.
    Tuple student({
        int64_t{1},
        std::string{"Alice"},
        int64_t{90}
    });

    // Construct the expression: score + 5
    AdditionExpression expression(
        std::make_unique<ColumnExpression>(2),
        std::make_unique<ConstantExpression>(int64_t{5})
    );

    // Evaluate against the student tuple.
    ExpressionValue result =
        expression.Evaluate(student);

    // Verify that the result is an INT64.
    ASSERT_TRUE(std::holds_alternative<int64_t>(result));

    // Verify that 90 + 5 = 95.
    EXPECT_EQ(std::get<int64_t>(result), 95);
}