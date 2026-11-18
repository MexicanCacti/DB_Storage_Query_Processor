
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
#include "not_equal_expression.hpp"

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
// Integer inequality
// --------------------------------------------------

TEST(NotEqualExpressionTest, UnequalPositiveIntegers)
{
    NotEqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualPositiveIntegers)
{
    NotEqualExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, UnequalNegativeIntegers)
{
    NotEqualExpression expression(
        IntConstant(-10),
        IntConstant(-20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualNegativeIntegers)
{
    NotEqualExpression expression(
        IntConstant(-10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, PositiveVersusNegative)
{
    NotEqualExpression expression(
        IntConstant(10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualZeros)
{
    NotEqualExpression expression(
        IntConstant(0),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, ZeroVersusNonzero)
{
    NotEqualExpression expression(
        IntConstant(0),
        IntConstant(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, NonzeroVersusZero)
{
    NotEqualExpression expression(
        IntConstant(1),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Integer boundary values
// --------------------------------------------------

TEST(NotEqualExpressionTest, EqualMaximumIntegers)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    NotEqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualMinimumIntegers)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    NotEqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, MaximumVersusMinimum)
{
    NotEqualExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, MaximumVersusMaximumMinusOne)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    NotEqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue - 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, MinimumVersusMinimumPlusOne)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    NotEqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue + 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// String inequality
// --------------------------------------------------

TEST(NotEqualExpressionTest, UnequalStrings)
{
    NotEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualStrings)
{
    NotEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, CaseSensitiveComparison)
{
    NotEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualEmptyStrings)
{
    NotEqualExpression expression(
        StringConstant(""),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EmptyVersusNonemptyString)
{
    NotEqualExpression expression(
        StringConstant(""),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, StringWhitespaceDifference)
{
    NotEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello ")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualUTF8Strings)
{
    NotEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, DifferentUTF8Strings)
{
    NotEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA8")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, UTF8VersusASCII)
{
    NotEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("e")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, MaximumLengthStringInequality)
{
    const std::string left(32, 'A');
    const std::string right(32, 'B');

    NotEqualExpression expression(
        StringConstant(left),
        StringConstant(right)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, MaximumLengthStringEquality)
{
    const std::string value(32, 'A');

    NotEqualExpression expression(
        StringConstant(value),
        StringConstant(value)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons
// --------------------------------------------------

TEST(NotEqualExpressionTest, ColumnNotEqualToConstant)
{
    NotEqualExpression expression(
        Column(0),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, ColumnEqualToConstant)
{
    NotEqualExpression expression(
        Column(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, ConstantNotEqualToColumn)
{
    NotEqualExpression expression(
        IntConstant(5),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, ConstantEqualToColumn)
{
    NotEqualExpression expression(
        IntConstant(-5),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, UnequalIntegerColumns)
{
    NotEqualExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualIntegerColumns)
{
    NotEqualExpression expression(
        Column(0),
        Column(4)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, UnequalStringColumns)
{
    NotEqualExpression expression(
        Column(1),
        Column(3)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualStringColumns)
{
    NotEqualExpression expression(
        Column(1),
        Column(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, StringColumnNotEqualToConstant)
{
    NotEqualExpression expression(
        Column(1),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, StringColumnEqualToConstant)
{
    NotEqualExpression expression(
        Column(1),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, SameColumnComparedToItself)
{
    NotEqualExpression expression(
        Column(0),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested arithmetic expressions
// --------------------------------------------------

TEST(NotEqualExpressionTest, EqualArithmeticResults)
{
    // (10 + 5) != (20 - 5)
    // 15 != 15 -> false

    NotEqualExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, UnequalArithmeticResults)
{
    // (10 + 5) != (20 - 10)
    // 15 != 10 -> true

    NotEqualExpression expression(
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
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, ColumnArithmeticComparison)
{
    // (Column 0 + Column 2) != 10
    // (10 + -5) != 10 -> true

    NotEqualExpression expression(
        std::make_unique<AdditionExpression>(
            Column(0),
            Column(2)
        ),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested boolean comparisons
// --------------------------------------------------

TEST(NotEqualExpressionTest, DifferentBooleanResults)
{
    // (10 == 10) != (20 == 30)
    // true != false -> true

    NotEqualExpression expression(
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
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualTrueBooleanResults)
{
    // true != true -> false

    NotEqualExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, EqualFalseBooleanResults)
{
    // false != false -> false

    NotEqualExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotEqualExpressionTest, NestedNotEqualExpressions)
{
    // (10 != 20) != (30 != 30)
    // true != false -> true

    NotEqualExpression expression(
        std::make_unique<NotEqualExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<NotEqualExpression>(
            IntConstant(30),
            IntConstant(30)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(NotEqualExpressionTest, IntegerComparedWithString)
{
    NotEqualExpression expression(
        IntConstant(10),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, StringComparedWithInteger)
{
    NotEqualExpression expression(
        StringConstant("10"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, IntegerColumnComparedWithStringColumn)
{
    NotEqualExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, StringColumnComparedWithIntegerConstant)
{
    NotEqualExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, IntegerColumnComparedWithStringConstant)
{
    NotEqualExpression expression(
        Column(0),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, BooleanComparedWithInteger)
{
    NotEqualExpression expression(
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

TEST(NotEqualExpressionTest, BooleanComparedWithString)
{
    NotEqualExpression expression(
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

TEST(NotEqualExpressionTest, ColumnIndexOutOfBounds)
{
    NotEqualExpression expression(
        Column(6),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, VeryLargeColumnIndex)
{
    NotEqualExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, InvalidRightColumn)
{
    NotEqualExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotEqualExpressionTest, BothColumnsInvalid)
{
    NotEqualExpression expression(
        Column(100),
        Column(200)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Input row immutability
// --------------------------------------------------

TEST(NotEqualExpressionTest, EvaluationDoesNotModifyRow)
{
    NotEqualExpression expression(
        Column(0),
        Column(2)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(NotEqualExpressionTest, StringEvaluationDoesNotModifyRow)
{
    NotEqualExpression expression(
        Column(1),
        Column(5)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

// --------------------------------------------------
// Boolean return type
// --------------------------------------------------

TEST(NotEqualExpressionTest, IntegerComparisonReturnsBoolean)
{
    NotEqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(NotEqualExpressionTest, StringComparisonReturnsBoolean)
{
    NotEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

// --------------------------------------------------
// Symmetry of inequality
// --------------------------------------------------

TEST(NotEqualExpressionTest, IntegerComparisonIsSymmetric)
{
    NotEqualExpression leftToRight(
        IntConstant(10),
        IntConstant(20)
    );

    NotEqualExpression rightToLeft(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_EQ(
        std::get<bool>(result1),
        std::get<bool>(result2)
    );

    EXPECT_TRUE(std::get<bool>(result1));
}

TEST(NotEqualExpressionTest, StringComparisonIsSymmetric)
{
    NotEqualExpression leftToRight(
        StringConstant("Hello"),
        StringConstant("World")
    );

    NotEqualExpression rightToLeft(
        StringConstant("World"),
        StringConstant("Hello")
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_EQ(
        std::get<bool>(result1),
        std::get<bool>(result2)
    );

    EXPECT_TRUE(std::get<bool>(result1));
}

// --------------------------------------------------
// Relationship with EqualExpression
// --------------------------------------------------

TEST(NotEqualExpressionTest, OppositeOfEqualForIntegers)
{
    EqualExpression equalExpression(
        IntConstant(10),
        IntConstant(20)
    );

    NotEqualExpression notEqualExpression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant equalResult =
        equalExpression.Eval(testRow);

    ExpressionVariant notEqualResult =
        notEqualExpression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(equalResult));
    ASSERT_TRUE(std::holds_alternative<bool>(notEqualResult));

    EXPECT_EQ(
        std::get<bool>(notEqualResult),
        !std::get<bool>(equalResult)
    );
}

TEST(NotEqualExpressionTest, OppositeOfEqualForStrings)
{
    EqualExpression equalExpression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    NotEqualExpression notEqualExpression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant equalResult =
        equalExpression.Eval(testRow);

    ExpressionVariant notEqualResult =
        notEqualExpression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(equalResult));
    ASSERT_TRUE(std::holds_alternative<bool>(notEqualResult));

    EXPECT_EQ(
        std::get<bool>(notEqualResult),
        !std::get<bool>(equalResult)
    );
}
