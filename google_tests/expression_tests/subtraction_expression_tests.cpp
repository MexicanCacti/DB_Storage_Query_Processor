
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include "expression.hpp"
#include "constant_expression.hpp"
#include "column_expression.hpp"
#include "subtraction_expression.hpp"

// Test tuple:
// Column 0: INT64  -> 10
// Column 1: STRING -> "Hello"
// Column 2: INT64  -> -5
// Column 3: STRING -> ""

static const Tuple testTuple({
    int64_t{10},
    std::string{"Hello"},
    int64_t{-5},
    std::string{""}
});

static const Row testRow{testTuple};

// Helper functions to construct expressions.

static std::unique_ptr<Expression> IntConstant(int64_t value)
{
    return std::make_unique<ConstantExpression>(value);
}

static std::unique_ptr<Expression> StringConstant(
    const std::string& value)
{
    return std::make_unique<ConstantExpression>(value);
}

static std::unique_ptr<Expression> Column(std::size_t index)
{
    return std::make_unique<ColumnExpression>(index);
}

// --------------------------------------------------
// Valid integer arithmetic
// --------------------------------------------------

TEST(SubtractionExpressionTest, PositiveIntegerSubtraction)
{
    SubtractionExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 10);
}

TEST(SubtractionExpressionTest, NegativeIntegerSubtraction)
{
    // -10 - (-20) = 10

    SubtractionExpression expression(
        IntConstant(-10),
        IntConstant(-20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 10);
}

TEST(SubtractionExpressionTest, MixedSignSubtraction)
{
    // 20 - (-5) = 25

    SubtractionExpression expression(
        IntConstant(20),
        IntConstant(-5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 25);
}

TEST(SubtractionExpressionTest, NegativeMinusPositive)
{
    // -20 - 5 = -25

    SubtractionExpression expression(
        IntConstant(-20),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), -25);
}

TEST(SubtractionExpressionTest, SubtractionWithZero)
{
    SubtractionExpression expression(
        IntConstant(25),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 25);
}

TEST(SubtractionExpressionTest, ZeroMinusPositive)
{
    SubtractionExpression expression(
        IntConstant(0),
        IntConstant(25)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), -25);
}

TEST(SubtractionExpressionTest, SubtractionResultZero)
{
    SubtractionExpression expression(
        IntConstant(25),
        IntConstant(25)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 0);
}

TEST(SubtractionExpressionTest, ColumnMinusConstant)
{
    // 10 - 5 = 5

    SubtractionExpression expression(
        Column(0),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 5);
}

TEST(SubtractionExpressionTest, ConstantMinusColumn)
{
    // 5 - (-5) = 10

    SubtractionExpression expression(
        IntConstant(5),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 10);
}

TEST(SubtractionExpressionTest, ColumnMinusColumn)
{
    // 10 - (-5) = 15

    SubtractionExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 15);
}

TEST(SubtractionExpressionTest, NestedSubtraction)
{
    // (30 - 10) - (-5) = 25

    SubtractionExpression expression(
        std::make_unique<SubtractionExpression>(
            IntConstant(30),
            IntConstant(10)
        ),
        IntConstant(-5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 25);
}

TEST(SubtractionExpressionTest, RightNestedSubtraction)
{
    // 30 - (10 - 5) = 25

    SubtractionExpression expression(
        IntConstant(30),
        std::make_unique<SubtractionExpression>(
            IntConstant(10),
            IntConstant(5)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 25);
}

// --------------------------------------------------
// Invalid column positions
// --------------------------------------------------

TEST(SubtractionExpressionTest, ColumnIndexOutOfBounds)
{
    SubtractionExpression expression(
        Column(4),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(SubtractionExpressionTest, VeryLargeColumnIndex)
{
    SubtractionExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(SubtractionExpressionTest, InvalidRightColumn)
{
    SubtractionExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(SubtractionExpressionTest, IntegerMinusString)
{
    SubtractionExpression expression(
        IntConstant(10),
        StringConstant("Hello")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(SubtractionExpressionTest, StringMinusInteger)
{
    SubtractionExpression expression(
        StringConstant("Hello"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(SubtractionExpressionTest, StringMinusString)
{
    SubtractionExpression expression(
        StringConstant("Hello"),
        StringConstant("World")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(SubtractionExpressionTest, StringColumnMinusInteger)
{
    SubtractionExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(SubtractionExpressionTest, IntegerColumnMinusStringColumn)
{
    SubtractionExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Signed integer overflow and underflow
// --------------------------------------------------

TEST(SubtractionExpressionTest, MaximumIntegerMinusNegativeOne)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(-1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, MinimumIntegerMinusPositiveOne)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, PositiveOverflow)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max() - 5),
        IntConstant(-10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, NegativeUnderflow)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min() + 5),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, ZeroMinusMinimumInteger)
{
    SubtractionExpression expression(
        IntConstant(0),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, MaximumIntegerMinusMinimumInteger)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, MinimumIntegerMinusMaximumInteger)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(std::numeric_limits<int64_t>::max())
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

// --------------------------------------------------
// Valid integer boundary conditions
// --------------------------------------------------

TEST(SubtractionExpressionTest, MaximumIntegerMinusZero)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    SubtractionExpression expression(
        IntConstant(maxValue),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), maxValue);
}

TEST(SubtractionExpressionTest, MinimumIntegerMinusZero)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    SubtractionExpression expression(
        IntConstant(minValue),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), minValue);
}

TEST(SubtractionExpressionTest, MaximumRepresentableResult)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    SubtractionExpression expression(
        IntConstant(maxValue - 1),
        IntConstant(-1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), maxValue);
}

TEST(SubtractionExpressionTest, MinimumRepresentableResult)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    SubtractionExpression expression(
        IntConstant(minValue + 1),
        IntConstant(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), minValue);
}

TEST(SubtractionExpressionTest, MaximumMinusMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    SubtractionExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 0);
}

TEST(SubtractionExpressionTest, MinimumMinusMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    SubtractionExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 0);
}

// --------------------------------------------------
// Input row immutability
// --------------------------------------------------

TEST(SubtractionExpressionTest, EvaluationDoesNotModifyRow)
{
    SubtractionExpression expression(
        Column(0),
        Column(2)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

// --------------------------------------------------
// Clear error reporting
// --------------------------------------------------

TEST(SubtractionExpressionTest, OverflowProvidesClearError)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(-1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(SubtractionExpressionTest, UnderflowProvidesClearError)
{
    SubtractionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}
