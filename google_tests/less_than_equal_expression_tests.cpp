
#include <gtest/gtest.h>

#include <cstdint>
#include <cstddef>
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
#include "less_than_equal_expression.hpp"

// Test tuple:
// Column 0: INT64  -> 10
// Column 1: STRING -> "Hello"
// Column 2: INT64  -> -5
// Column 3: STRING -> ""
// Column 4: INT64  -> 10
// Column 5: STRING -> "World"
// Column 6: INT64  -> 0
// Column 7: STRING -> "Hello"

static const Tuple testTuple({
    int64_t{10},
    std::string{"Hello"},
    int64_t{-5},
    std::string{""},
    int64_t{10},
    std::string{"World"},
    int64_t{0},
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

static std::unique_ptr<Expression> BoolExpression(bool value)
{
    if (value)
    {
        return std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
        );
    }

    return std::make_unique<EqualExpression>(
        IntConstant(10),
        IntConstant(20)
    );
}

// --------------------------------------------------
// Integer comparisons
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, SmallerPositiveIntegers)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, GreaterPositiveIntegers)
{
    LessThanEqualExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualPositiveIntegers)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, SmallerNegativeIntegers)
{
    // -20 <= -10

    LessThanEqualExpression expression(
        IntConstant(-20),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, GreaterNegativeIntegers)
{
    // -5 <= -10

    LessThanEqualExpression expression(
        IntConstant(-5),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualNegativeIntegers)
{
    LessThanEqualExpression expression(
        IntConstant(-10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, NegativeLessThanPositive)
{
    LessThanEqualExpression expression(
        IntConstant(-10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, PositiveLessThanNegative)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Zero comparisons
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, NegativeLessThanZero)
{
    LessThanEqualExpression expression(
        IntConstant(-10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ZeroLessThanPositive)
{
    LessThanEqualExpression expression(
        IntConstant(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ZeroLessThanNegative)
{
    LessThanEqualExpression expression(
        IntConstant(0),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, PositiveLessThanZero)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ZeroLessThanEqualZero)
{
    LessThanEqualExpression expression(
        IntConstant(0),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Integer boundary values
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, MinimumLessThanMaximum)
{
    LessThanEqualExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(std::numeric_limits<int64_t>::max())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MaximumLessThanMinimum)
{
    LessThanEqualExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MaximumEqualMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    LessThanEqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MinimumEqualMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    LessThanEqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MaximumMinusOneLessThanMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    LessThanEqualExpression expression(
        IntConstant(maxValue - 1),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MinimumLessThanMinimumPlusOne)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    LessThanEqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue + 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MaximumNotLessThanMaximumMinusOne)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    LessThanEqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue - 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MinimumPlusOneNotLessThanMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    LessThanEqualExpression expression(
        IntConstant(minValue + 1),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String comparisons - ASCII
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, LexicographicallySmallerString)
{
    LessThanEqualExpression expression(
        StringConstant("Apple"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, LexicographicallyGreaterString)
{
    LessThanEqualExpression expression(
        StringConstant("World"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualStrings)
{
    LessThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, FirstCharacterDeterminesOrder)
{
    LessThanEqualExpression expression(
        StringConstant("Apple"),
        StringConstant("Zebra")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, LaterCharacterDeterminesOrder)
{
    // "abc" <= "abd"

    LessThanEqualExpression expression(
        StringConstant("abc"),
        StringConstant("abd")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EarlierCharacterTakesPrecedence)
{
    // "az" <= "za"

    LessThanEqualExpression expression(
        StringConstant("az"),
        StringConstant("za")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, LaterGreaterCharacterCannotOverrideEarlierSmaller)
{
    // "abz" <= "aca"

    LessThanEqualExpression expression(
        StringConstant("abz"),
        StringConstant("aca")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, FirstDifferingCharacterDeterminesOrder)
{
    // "abz" <= "aby" -> false

    LessThanEqualExpression expression(
        StringConstant("abz"),
        StringConstant("aby")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, UppercaseLessThanLowercase)
{
    LessThanEqualExpression expression(
        StringConstant("Apple"),
        StringConstant("apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, LowercaseGreaterThanUppercase)
{
    LessThanEqualExpression expression(
        StringConstant("apple"),
        StringConstant("Apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String prefix and length comparisons
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, ShorterStringWithEqualPrefix)
{
    LessThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("HelloWorld")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, LongerStringWithEqualPrefix)
{
    LessThanEqualExpression expression(
        StringConstant("HelloWorld"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualLengthStrings)
{
    LessThanEqualExpression expression(
        StringConstant("abc"),
        StringConstant("abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EmptyStringVersusEmptyString)
{
    LessThanEqualExpression expression(
        StringConstant(""),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EmptyStringLessThanNonempty)
{
    LessThanEqualExpression expression(
        StringConstant(""),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, NonemptyStringNotLessThanEmpty)
{
    LessThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, LongerStringCanBeSmaller)
{
    // "aaa" <= "b"

    LessThanEqualExpression expression(
        StringConstant("aaa"),
        StringConstant("b")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ShorterStringNotAlwaysSmaller)
{
    // "z" <= "aaaa"

    LessThanEqualExpression expression(
        StringConstant("z"),
        StringConstant("aaaa")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// UTF-8 comparisons
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, EqualUTF8Strings)
{
    LessThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, SmallerUTF8String)
{
    // è <= é

    LessThanEqualExpression expression(
        StringConstant("\xC3\xA8"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, GreaterUTF8String)
{
    // é <= è

    LessThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA8")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ASCIILessThanUTF8)
{
    LessThanEqualExpression expression(
        StringConstant("e"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, UTF8GreaterThanASCII)
{
    LessThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("e")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, UTF8PrefixComparison)
{
    LessThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9" "abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons - integers
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, IntegerColumnLessThanConstant)
{
    LessThanEqualExpression expression(
        Column(0),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, IntegerColumnGreaterThanConstant)
{
    LessThanEqualExpression expression(
        Column(0),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, IntegerColumnEqualToConstant)
{
    LessThanEqualExpression expression(
        Column(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ConstantLessThanIntegerColumn)
{
    LessThanEqualExpression expression(
        IntConstant(5),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, IntegerColumnLessThanIntegerColumn)
{
    // -5 <= 10

    LessThanEqualExpression expression(
        Column(2),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, IntegerColumnGreaterThanIntegerColumn)
{
    // 10 <= -5

    LessThanEqualExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualIntegerColumns)
{
    LessThanEqualExpression expression(
        Column(0),
        Column(4)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, SameColumnComparedToItself)
{
    LessThanEqualExpression expression(
        Column(0),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons - strings
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, StringColumnLessThanConstant)
{
    LessThanEqualExpression expression(
        Column(1),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, StringColumnGreaterThanConstant)
{
    LessThanEqualExpression expression(
        Column(5),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, StringColumnEqualToConstant)
{
    LessThanEqualExpression expression(
        Column(1),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, StringColumnLessThanStringColumn)
{
    LessThanEqualExpression expression(
        Column(1),
        Column(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, StringColumnGreaterThanStringColumn)
{
    LessThanEqualExpression expression(
        Column(5),
        Column(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualStringColumns)
{
    LessThanEqualExpression expression(
        Column(1),
        Column(7)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EmptyStringColumnLessThanNonempty)
{
    LessThanEqualExpression expression(
        Column(3),
        Column(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested arithmetic expressions
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, AdditionLessThanConstant)
{
    // (10 + 5) <= 20

    LessThanEqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(5)
        ),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, SubtractionEqualToConstant)
{
    // (20 - 5) <= 15

    LessThanEqualExpression expression(
        std::make_unique<SubtractionExpression>(
            IntConstant(20),
            IntConstant(5)
        ),
        IntConstant(15)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, AdditionLessThanSubtraction)
{
    // (5 + 5) <= (20 - 5)

    LessThanEqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(5),
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

TEST(LessThanEqualExpressionTest, AdditionGreaterThanSubtraction)
{
    // (10 + 10) <= (20 - 5)

    LessThanEqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(10)
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

TEST(LessThanEqualExpressionTest, EqualArithmeticResults)
{
    // (10 + 5) <= (20 - 5)

    LessThanEqualExpression expression(
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

TEST(LessThanEqualExpressionTest, NestedArithmeticComparison)
{
    // ((20 - 5) + 10) <= (30 - 10)
    // 25 <= 20 -> false

    LessThanEqualExpression expression(
        std::make_unique<AdditionExpression>(
            std::make_unique<SubtractionExpression>(
                IntConstant(20),
                IntConstant(5)
            ),
            IntConstant(10)
        ),
        std::make_unique<SubtractionExpression>(
            IntConstant(30),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Overflow propagation
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, LeftAdditionOverflow)
{
    LessThanEqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(std::numeric_limits<int64_t>::max()),
            IntConstant(1)
        ),
        IntConstant(0)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(LessThanEqualExpressionTest, RightSubtractionUnderflow)
{
    LessThanEqualExpression expression(
        IntConstant(0),
        std::make_unique<SubtractionExpression>(
            IntConstant(std::numeric_limits<int64_t>::min()),
            IntConstant(1)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, IntegerComparedWithString)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, StringComparedWithInteger)
{
    LessThanEqualExpression expression(
        StringConstant("10"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, IntegerColumnComparedWithStringColumn)
{
    LessThanEqualExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, StringColumnComparedWithIntegerConstant)
{
    LessThanEqualExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, IntegerColumnComparedWithStringConstant)
{
    LessThanEqualExpression expression(
        Column(0),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, BooleanComparedWithInteger)
{
    LessThanEqualExpression expression(
        BoolExpression(true),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, IntegerComparedWithBoolean)
{
    LessThanEqualExpression expression(
        IntConstant(1),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, BooleanComparedWithString)
{
    LessThanEqualExpression expression(
        BoolExpression(true),
        StringConstant("true")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, BooleanComparedWithBoolean)
{
    LessThanEqualExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanEqualExpressionTest, EqualBooleansRejected)
{
    LessThanEqualExpression expression(
        BoolExpression(true),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

// --------------------------------------------------
// Invalid column positions
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, ColumnIndexOutOfBounds)
{
    LessThanEqualExpression expression(
        Column(8),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(LessThanEqualExpressionTest, VeryLargeColumnIndex)
{
    LessThanEqualExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(LessThanEqualExpressionTest, InvalidRightColumn)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(LessThanEqualExpressionTest, BothColumnsInvalid)
{
    LessThanEqualExpression expression(
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

TEST(LessThanEqualExpressionTest, IntegerEvaluationDoesNotModifyRow)
{
    LessThanEqualExpression expression(
        Column(2),
        Column(0)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(LessThanEqualExpressionTest, StringEvaluationDoesNotModifyRow)
{
    LessThanEqualExpression expression(
        Column(1),
        Column(5)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

// --------------------------------------------------
// Comparison properties
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, EqualIntegersAreLessThanEqual)
{
    LessThanEqualExpression expression(
        IntConstant(42),
        IntConstant(42)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualStringsAreLessThanEqual)
{
    LessThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, ReversingUnequalIntegersChangesResult)
{
    LessThanEqualExpression leftToRight(
        IntConstant(10),
        IntConstant(20)
    );

    LessThanEqualExpression rightToLeft(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_TRUE(std::get<bool>(result1));
    EXPECT_FALSE(std::get<bool>(result2));
}

TEST(LessThanEqualExpressionTest, ReversingUnequalStringsChangesResult)
{
    LessThanEqualExpression leftToRight(
        StringConstant("Hello"),
        StringConstant("World")
    );

    LessThanEqualExpression rightToLeft(
        StringConstant("World"),
        StringConstant("Hello")
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_TRUE(std::get<bool>(result1));
    EXPECT_FALSE(std::get<bool>(result2));
}

TEST(LessThanEqualExpressionTest, ReversingEqualIntegersBothTrue)
{
    LessThanEqualExpression leftToRight(
        IntConstant(10),
        IntConstant(10)
    );

    LessThanEqualExpression rightToLeft(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_TRUE(std::get<bool>(result1));
    EXPECT_TRUE(std::get<bool>(result2));
}

TEST(LessThanEqualExpressionTest, MaximumLengthStringComparison)
{
    const std::string left(32, 'A');
    const std::string right(32, 'Z');

    LessThanEqualExpression expression(
        StringConstant(left),
        StringConstant(right)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanEqualExpressionTest, MaximumLengthEqualStrings)
{
    const std::string value(32, 'A');

    LessThanEqualExpression expression(
        StringConstant(value),
        StringConstant(value)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Boolean return type
// --------------------------------------------------

TEST(LessThanEqualExpressionTest, TrueComparisonReturnsBoolean)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(LessThanEqualExpressionTest, FalseComparisonReturnsBoolean)
{
    LessThanEqualExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(LessThanEqualExpressionTest, EqualComparisonReturnsBoolean)
{
    LessThanEqualExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}
