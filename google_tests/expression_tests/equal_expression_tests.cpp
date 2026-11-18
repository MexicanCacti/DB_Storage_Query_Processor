
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
#include "subtraction_expression.hpp"
#include "equal_expression.hpp"

// Test tuple:
// Column 0: INT64  -> 10
// Column 1: STRING -> "Hello"
// Column 2: INT64  -> -5
// Column 3: STRING -> ""
// Column 4: INT64  -> 10
// Column 5: STRING -> "Hello"

static const Tuple testTuple({
    int64_t{10},
    std::string{"Hello"},
    int64_t{-5},
    std::string{""},
    int64_t{10},
    std::string{"Hello"}
});

static const Row testRow{testTuple};

// --------------------------------------------------
// Helper functions
// --------------------------------------------------

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
// Integer equality
// --------------------------------------------------

TEST(EqualExpressionTest, EqualPositiveIntegers)
{
    EqualExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UnequalPositiveIntegers)
{
    EqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualNegativeIntegers)
{
    EqualExpression expression(
        IntConstant(-10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UnequalNegativeIntegers)
{
    EqualExpression expression(
        IntConstant(-10),
        IntConstant(-20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, PositiveVersusNegative)
{
    EqualExpression expression(
        IntConstant(10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualZeros)
{
    EqualExpression expression(
        IntConstant(0),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, ZeroVersusNonzero)
{
    EqualExpression expression(
        IntConstant(0),
        IntConstant(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Integer boundary values
// --------------------------------------------------

TEST(EqualExpressionTest, EqualMaximumIntegers)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    EqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualMinimumIntegers)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    EqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, MaximumVersusMinimum)
{
    EqualExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, MaximumVersusMaximumMinusOne)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    EqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue - 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String equality
// --------------------------------------------------

TEST(EqualExpressionTest, EqualStrings)
{
    EqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UnequalStrings)
{
    EqualExpression expression(
        StringConstant("Hello"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, CaseSensitiveComparison)
{
    EqualExpression expression(
        StringConstant("Hello"),
        StringConstant("hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualEmptyStrings)
{
    EqualExpression expression(
        StringConstant(""),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EmptyVersusNonemptyString)
{
    EqualExpression expression(
        StringConstant(""),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, StringWhitespaceDifference)
{
    EqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello ")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualUTF8Strings)
{
    EqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, DifferentUTF8Strings)
{
    EqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA8")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UTF8VersusASCII)
{
    EqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("e")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, MaximumLengthStringEquality)
{
    const std::string value(32, 'A');

    EqualExpression expression(
        StringConstant(value),
        StringConstant(value)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons
// --------------------------------------------------

TEST(EqualExpressionTest, ColumnEqualsConstant)
{
    EqualExpression expression(
        Column(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, ColumnDoesNotEqualConstant)
{
    EqualExpression expression(
        Column(0),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, ConstantEqualsColumn)
{
    EqualExpression expression(
        IntConstant(-5),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualIntegerColumns)
{
    EqualExpression expression(
        Column(0),
        Column(4)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UnequalIntegerColumns)
{
    EqualExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, EqualStringColumns)
{
    EqualExpression expression(
        Column(1),
        Column(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UnequalStringColumns)
{
    EqualExpression expression(
        Column(1),
        Column(3)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, StringColumnEqualsConstant)
{
    EqualExpression expression(
        Column(1),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, SameColumnComparedToItself)
{
    EqualExpression expression(
        Column(0),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested expressions
// --------------------------------------------------

TEST(EqualExpressionTest, EqualAdditionResults)
{
    // (10 + 5) == (20 - 5)

    EqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(5)
        ),
        std::make_unique<SubtractionExpression>(
            IntConstant(20),
            IntConstant(5)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, UnequalArithmeticResults)
{
    // (10 + 5) == (20 - 10)

    EqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(5)
        ),
        std::make_unique<SubtractionExpression>(
            IntConstant(20),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(EqualExpressionTest, NestedEqualityReturnsBoolean)
{
    // (10 == 10) == (20 == 20)

    EqualExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
        ),
        std::make_unique<EqualExpression>(
            IntConstant(20),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, NestedEqualityFalseResults)
{
    // (10 == 20) == (30 == 40)

    EqualExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<EqualExpression>(
            IntConstant(30),
            IntConstant(40)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(EqualExpressionTest, NestedEqualityDifferentResults)
{
    // (10 == 10) == (20 == 30)

    EqualExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
        ),
        std::make_unique<EqualExpression>(
            IntConstant(20),
            IntConstant(30)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(EqualExpressionTest, IntegerComparedWithString)
{
    EqualExpression expression(
        IntConstant(10),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, StringComparedWithInteger)
{
    EqualExpression expression(
        StringConstant("10"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, IntegerColumnComparedWithStringColumn)
{
    EqualExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, StringColumnComparedWithIntegerConstant)
{
    EqualExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, IntegerColumnComparedWithStringConstant)
{
    EqualExpression expression(
        Column(0),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, BooleanComparedWithInteger)
{
    EqualExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
        ),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, BooleanComparedWithString)
{
    EqualExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
        ),
        StringConstant("true")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Invalid column positions
// --------------------------------------------------

TEST(EqualExpressionTest, ColumnIndexOutOfBounds)
{
    EqualExpression expression(
        Column(6),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, VeryLargeColumnIndex)
{
    EqualExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(EqualExpressionTest, InvalidRightColumn)
{
    EqualExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Input row immutability
// --------------------------------------------------

TEST(EqualExpressionTest, EvaluationDoesNotModifyRow)
{
    EqualExpression expression(
        Column(0),
        Column(4)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}
