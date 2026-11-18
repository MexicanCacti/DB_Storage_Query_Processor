
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
#include "addition_expression.hpp"

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

TEST(AdditionExpressionTest, PositiveIntegerAddition)
{
    AdditionExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 30);
}

TEST(AdditionExpressionTest, NegativeIntegerAddition)
{
    AdditionExpression expression(
        IntConstant(-10),
        IntConstant(-20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), -30);
}

TEST(AdditionExpressionTest, MixedSignAddition)
{
    AdditionExpression expression(
        IntConstant(20),
        IntConstant(-5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 15);
}

TEST(AdditionExpressionTest, AdditionWithZero)
{
    AdditionExpression expression(
        IntConstant(0),
        IntConstant(25)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 25);
}

TEST(AdditionExpressionTest, ColumnPlusConstant)
{
    AdditionExpression expression(
        Column(0),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 15);
}

TEST(AdditionExpressionTest, ConstantPlusColumn)
{
    AdditionExpression expression(
        IntConstant(5),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 0);
}

TEST(AdditionExpressionTest, ColumnPlusColumn)
{
    AdditionExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 5);
}

TEST(AdditionExpressionTest, NestedAddition)
{
    // (10 + 20) + (-5) = 25

    AdditionExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        IntConstant(-5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), 25);
}

// --------------------------------------------------
// Invalid column positions
// --------------------------------------------------

TEST(AdditionExpressionTest, ColumnIndexOutOfBounds)
{
    AdditionExpression expression(
        Column(4),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AdditionExpressionTest, VeryLargeColumnIndex)
{
    AdditionExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AdditionExpressionTest, InvalidRightColumn)
{
    AdditionExpression expression(
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

TEST(AdditionExpressionTest, IntegerPlusString)
{
    AdditionExpression expression(
        IntConstant(10),
        StringConstant("Hello")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AdditionExpressionTest, StringPlusInteger)
{
    AdditionExpression expression(
        StringConstant("Hello"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AdditionExpressionTest, StringPlusString)
{
    AdditionExpression expression(
        StringConstant("Hello"),
        StringConstant("World")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AdditionExpressionTest, StringColumnPlusInteger)
{
    AdditionExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AdditionExpressionTest, IntegerColumnPlusStringColumn)
{
    AdditionExpression expression(
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

TEST(AdditionExpressionTest, MaximumIntegerPlusOne)
{
    AdditionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(AdditionExpressionTest, MinimumIntegerMinusOne)
{
    AdditionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(-1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(AdditionExpressionTest, PositiveOverflow)
{
    AdditionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max() - 5),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(AdditionExpressionTest, NegativeUnderflow)
{
    AdditionExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min() + 5),
        IntConstant(-10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(AdditionExpressionTest, MaximumIntegerPlusZero)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    AdditionExpression expression(
        IntConstant(maxValue),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), maxValue);
}

TEST(AdditionExpressionTest, MinimumIntegerPlusZero)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    AdditionExpression expression(
        IntConstant(minValue),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), minValue);
}

TEST(AdditionExpressionTest, MaximumRepresentableResult)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    AdditionExpression expression(
        IntConstant(maxValue - 1),
        IntConstant(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), maxValue);
}

TEST(AdditionExpressionTest, MinimumRepresentableResult)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    AdditionExpression expression(
        IntConstant(minValue + 1),
        IntConstant(-1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<int64_t>(result));
    EXPECT_EQ(std::get<int64_t>(result), minValue);
}

// --------------------------------------------------
// Input row immutability
// --------------------------------------------------

TEST(AdditionExpressionTest, EvaluationDoesNotModifyRow)
{
    AdditionExpression expression(
        Column(0),
        Column(2)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}
