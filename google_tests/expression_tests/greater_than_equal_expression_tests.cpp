
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
#include "greater_than_equal_expression.hpp"

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

TEST(GreaterThanEqualExpressionTest, GreaterPositiveIntegers)
{
    GreaterThanEqualExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, SmallerPositiveIntegers)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualPositiveIntegers)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, GreaterNegativeIntegers)
{
    GreaterThanEqualExpression expression(
        IntConstant(-5),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, SmallerNegativeIntegers)
{
    GreaterThanEqualExpression expression(
        IntConstant(-20),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualNegativeIntegers)
{
    GreaterThanEqualExpression expression(
        IntConstant(-10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, PositiveGreaterThanNegative)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, NegativeGreaterThanPositive)
{
    GreaterThanEqualExpression expression(
        IntConstant(-10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Zero comparisons
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, PositiveGreaterThanZero)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ZeroGreaterThanPositive)
{
    GreaterThanEqualExpression expression(
        IntConstant(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ZeroGreaterThanNegative)
{
    GreaterThanEqualExpression expression(
        IntConstant(0),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, NegativeGreaterThanZero)
{
    GreaterThanEqualExpression expression(
        IntConstant(-10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ZeroGreaterThanEqualZero)
{
    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, MaximumGreaterThanMinimum)
{
    GreaterThanEqualExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MinimumGreaterThanMaximum)
{
    GreaterThanEqualExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(std::numeric_limits<int64_t>::max())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MaximumEqualMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    GreaterThanEqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MinimumEqualMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    GreaterThanEqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MaximumGreaterThanMaximumMinusOne)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    GreaterThanEqualExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue - 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MinimumPlusOneGreaterThanMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    GreaterThanEqualExpression expression(
        IntConstant(minValue + 1),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MaximumMinusOneNotGreaterThanMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    GreaterThanEqualExpression expression(
        IntConstant(maxValue - 1),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MinimumNotGreaterThanMinimumPlusOne)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    GreaterThanEqualExpression expression(
        IntConstant(minValue),
        IntConstant(minValue + 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String comparisons - ASCII
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, LexicographicallyGreaterString)
{
    GreaterThanEqualExpression expression(
        StringConstant("World"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, LexicographicallySmallerString)
{
    GreaterThanEqualExpression expression(
        StringConstant("Apple"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualStrings)
{
    GreaterThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, FirstCharacterDeterminesOrder)
{
    GreaterThanEqualExpression expression(
        StringConstant("Zebra"),
        StringConstant("Apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, LaterCharacterDeterminesOrder)
{
    // "abd" >= "abc"

    GreaterThanEqualExpression expression(
        StringConstant("abd"),
        StringConstant("abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EarlierCharacterTakesPrecedence)
{
    // "az" < "za"

    GreaterThanEqualExpression expression(
        StringConstant("az"),
        StringConstant("za")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, LaterGreaterCharacterCannotOverrideEarlierSmaller)
{
    // "abz" < "aca"

    GreaterThanEqualExpression expression(
        StringConstant("abz"),
        StringConstant("aca")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, FirstDifferingCharacterDeterminesOrder)
{
    GreaterThanEqualExpression expression(
        StringConstant("abz"),
        StringConstant("aby")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, LowercaseGreaterThanUppercase)
{
    GreaterThanEqualExpression expression(
        StringConstant("apple"),
        StringConstant("Apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, UppercaseSmallerThanLowercase)
{
    GreaterThanEqualExpression expression(
        StringConstant("Apple"),
        StringConstant("apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String prefix and length comparisons
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, LongerStringWithEqualPrefix)
{
    GreaterThanEqualExpression expression(
        StringConstant("HelloWorld"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ShorterStringWithEqualPrefix)
{
    GreaterThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("HelloWorld")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualLengthStrings)
{
    GreaterThanEqualExpression expression(
        StringConstant("abc"),
        StringConstant("abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EmptyStringVersusEmptyString)
{
    GreaterThanEqualExpression expression(
        StringConstant(""),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, NonemptyStringGreaterThanEmpty)
{
    GreaterThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EmptyStringNotGreaterThanNonempty)
{
    GreaterThanEqualExpression expression(
        StringConstant(""),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, LongerStringNotAlwaysGreater)
{
    GreaterThanEqualExpression expression(
        StringConstant("aaa"),
        StringConstant("b")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ShorterStringCanBeGreater)
{
    GreaterThanEqualExpression expression(
        StringConstant("z"),
        StringConstant("aaaa")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// UTF-8 comparisons
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, EqualUTF8Strings)
{
    GreaterThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, GreaterUTF8String)
{
    // UTF-8 é >= è

    GreaterThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA8")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, SmallerUTF8String)
{
    // UTF-8 è >= é

    GreaterThanEqualExpression expression(
        StringConstant("\xC3\xA8"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, UTF8GreaterThanASCII)
{
    GreaterThanEqualExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("e")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ASCIISmallerThanUTF8)
{
    GreaterThanEqualExpression expression(
        StringConstant("e"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, UTF8PrefixComparison)
{
    GreaterThanEqualExpression expression(
        StringConstant("\xC3\xA9" "abc"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons - integers
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, IntegerColumnGreaterThanConstant)
{
    GreaterThanEqualExpression expression(
        Column(0),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, IntegerColumnSmallerThanConstant)
{
    GreaterThanEqualExpression expression(
        Column(0),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, IntegerColumnEqualToConstant)
{
    GreaterThanEqualExpression expression(
        Column(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ConstantGreaterThanIntegerColumn)
{
    GreaterThanEqualExpression expression(
        IntConstant(20),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, IntegerColumnGreaterThanIntegerColumn)
{
    GreaterThanEqualExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, IntegerColumnSmallerThanIntegerColumn)
{
    GreaterThanEqualExpression expression(
        Column(2),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualIntegerColumns)
{
    GreaterThanEqualExpression expression(
        Column(0),
        Column(4)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, SameColumnComparedToItself)
{
    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, StringColumnGreaterThanConstant)
{
    GreaterThanEqualExpression expression(
        Column(5),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, StringColumnSmallerThanConstant)
{
    GreaterThanEqualExpression expression(
        Column(1),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, StringColumnEqualToConstant)
{
    GreaterThanEqualExpression expression(
        Column(1),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, StringColumnGreaterThanStringColumn)
{
    GreaterThanEqualExpression expression(
        Column(5),
        Column(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, StringColumnSmallerThanStringColumn)
{
    GreaterThanEqualExpression expression(
        Column(1),
        Column(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualStringColumns)
{
    GreaterThanEqualExpression expression(
        Column(1),
        Column(7)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, NonemptyStringColumnGreaterThanEmpty)
{
    GreaterThanEqualExpression expression(
        Column(1),
        Column(3)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested arithmetic expressions
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, AdditionGreaterThanConstant)
{
    GreaterThanEqualExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(5)
        ),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, SubtractionEqualToConstant)
{
    // (20 - 5) >= 15

    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, AdditionGreaterThanSubtraction)
{
    // (10 + 10) >= (20 - 5)

    GreaterThanEqualExpression expression(
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
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, AdditionSmallerThanSubtraction)
{
    // (5 + 5) >= (20 - 5)

    GreaterThanEqualExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualArithmeticResults)
{
    // (10 + 5) >= (20 - 5)

    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, NestedArithmeticComparison)
{
    // ((20 - 5) + 10) >= (30 - 10)

    GreaterThanEqualExpression expression(
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
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Overflow propagation
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, LeftAdditionOverflow)
{
    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, RightSubtractionUnderflow)
{
    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, IntegerComparedWithString)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, StringComparedWithInteger)
{
    GreaterThanEqualExpression expression(
        StringConstant("10"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, IntegerColumnComparedWithStringColumn)
{
    GreaterThanEqualExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, StringColumnComparedWithIntegerConstant)
{
    GreaterThanEqualExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, IntegerColumnComparedWithStringConstant)
{
    GreaterThanEqualExpression expression(
        Column(0),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, BooleanComparedWithInteger)
{
    GreaterThanEqualExpression expression(
        BoolExpression(true),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, IntegerComparedWithBoolean)
{
    GreaterThanEqualExpression expression(
        IntConstant(1),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, BooleanComparedWithString)
{
    GreaterThanEqualExpression expression(
        BoolExpression(true),
        StringConstant("true")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, BooleanComparedWithBoolean)
{
    GreaterThanEqualExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanEqualExpressionTest, EqualBooleansRejected)
{
    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, ColumnIndexOutOfBounds)
{
    GreaterThanEqualExpression expression(
        Column(8),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(GreaterThanEqualExpressionTest, VeryLargeColumnIndex)
{
    GreaterThanEqualExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(GreaterThanEqualExpressionTest, InvalidRightColumn)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(GreaterThanEqualExpressionTest, BothColumnsInvalid)
{
    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, IntegerEvaluationDoesNotModifyRow)
{
    GreaterThanEqualExpression expression(
        Column(0),
        Column(2)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(GreaterThanEqualExpressionTest, StringEvaluationDoesNotModifyRow)
{
    GreaterThanEqualExpression expression(
        Column(5),
        Column(1)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

// --------------------------------------------------
// Comparison properties
// --------------------------------------------------

TEST(GreaterThanEqualExpressionTest, EqualIntegersAreGreaterThanEqual)
{
    GreaterThanEqualExpression expression(
        IntConstant(42),
        IntConstant(42)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualStringsAreGreaterThanEqual)
{
    GreaterThanEqualExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, ReversingUnequalIntegersChangesResult)
{
    GreaterThanEqualExpression leftToRight(
        IntConstant(20),
        IntConstant(10)
    );

    GreaterThanEqualExpression rightToLeft(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_TRUE(std::get<bool>(result1));
    EXPECT_FALSE(std::get<bool>(result2));
}

TEST(GreaterThanEqualExpressionTest, ReversingUnequalStringsChangesResult)
{
    GreaterThanEqualExpression leftToRight(
        StringConstant("World"),
        StringConstant("Hello")
    );

    GreaterThanEqualExpression rightToLeft(
        StringConstant("Hello"),
        StringConstant("World")
    );

    ExpressionVariant result1 = leftToRight.Eval(testRow);
    ExpressionVariant result2 = rightToLeft.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_TRUE(std::get<bool>(result1));
    EXPECT_FALSE(std::get<bool>(result2));
}

TEST(GreaterThanEqualExpressionTest, ReversingEqualIntegersBothTrue)
{
    GreaterThanEqualExpression leftToRight(
        IntConstant(10),
        IntConstant(10)
    );

    GreaterThanEqualExpression rightToLeft(
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

TEST(GreaterThanEqualExpressionTest, MaximumLengthStringComparison)
{
    const std::string left(32, 'Z');
    const std::string right(32, 'A');

    GreaterThanEqualExpression expression(
        StringConstant(left),
        StringConstant(right)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, MaximumLengthEqualStrings)
{
    const std::string value(32, 'A');

    GreaterThanEqualExpression expression(
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

TEST(GreaterThanEqualExpressionTest, TrueComparisonReturnsBoolean)
{
    GreaterThanEqualExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, FalseComparisonReturnsBoolean)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(GreaterThanEqualExpressionTest, EqualComparisonReturnsBoolean)
{
    GreaterThanEqualExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}
