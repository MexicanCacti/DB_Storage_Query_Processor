
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
#include "less_than_expression.hpp"

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

TEST(LessThanExpressionTest, SmallerPositiveIntegers)
{
    LessThanExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, GreaterPositiveIntegers)
{
    LessThanExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EqualPositiveIntegers)
{
    LessThanExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, SmallerNegativeIntegers)
{
    // -20 < -10

    LessThanExpression expression(
        IntConstant(-20),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, GreaterNegativeIntegers)
{
    // -5 < -10

    LessThanExpression expression(
        IntConstant(-5),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EqualNegativeIntegers)
{
    LessThanExpression expression(
        IntConstant(-10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, NegativeLessThanPositive)
{
    LessThanExpression expression(
        IntConstant(-10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, PositiveLessThanNegative)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, NegativeLessThanZero)
{
    LessThanExpression expression(
        IntConstant(-10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, ZeroLessThanPositive)
{
    LessThanExpression expression(
        IntConstant(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, ZeroLessThanNegative)
{
    LessThanExpression expression(
        IntConstant(0),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, PositiveLessThanZero)
{
    LessThanExpression expression(
        IntConstant(10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, ZeroLessThanZero)
{
    LessThanExpression expression(
        IntConstant(0),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Integer boundary values
// --------------------------------------------------

TEST(LessThanExpressionTest, MinimumLessThanMaximum)
{
    LessThanExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(std::numeric_limits<int64_t>::max())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, MaximumLessThanMinimum)
{
    LessThanExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, MaximumLessThanMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    LessThanExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, MinimumLessThanMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    LessThanExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, MaximumMinusOneLessThanMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    LessThanExpression expression(
        IntConstant(maxValue - 1),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, MinimumLessThanMinimumPlusOne)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    LessThanExpression expression(
        IntConstant(minValue),
        IntConstant(minValue + 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// String comparisons - ASCII
// --------------------------------------------------

TEST(LessThanExpressionTest, LexicographicallySmallerString)
{
    LessThanExpression expression(
        StringConstant("Apple"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, LexicographicallyGreaterString)
{
    LessThanExpression expression(
        StringConstant("World"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EqualStrings)
{
    LessThanExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, FirstCharacterDeterminesOrder)
{
    LessThanExpression expression(
        StringConstant("Apple"),
        StringConstant("Zebra")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, LaterCharacterDeterminesOrder)
{
    // "abc" < "abd"

    LessThanExpression expression(
        StringConstant("abc"),
        StringConstant("abd")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EarlierCharacterTakesPrecedence)
{
    // "az" < "za"

    LessThanExpression expression(
        StringConstant("az"),
        StringConstant("za")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, LaterGreaterCharacterCannotOverrideEarlierSmaller)
{
    // "abz" < "aca"

    LessThanExpression expression(
        StringConstant("abz"),
        StringConstant("aca")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, FirstDifferingCharacterDeterminesOrder)
{
    // "abz" < "aby" -> false

    LessThanExpression expression(
        StringConstant("abz"),
        StringConstant("aby")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, UppercaseVersusLowercase)
{
    // ASCII 'A' < 'a'

    LessThanExpression expression(
        StringConstant("Apple"),
        StringConstant("apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, LowercaseComparedWithUppercase)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, ShorterStringWithEqualPrefix)
{
    LessThanExpression expression(
        StringConstant("Hello"),
        StringConstant("HelloWorld")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, LongerStringWithEqualPrefix)
{
    LessThanExpression expression(
        StringConstant("HelloWorld"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EqualLengthStrings)
{
    LessThanExpression expression(
        StringConstant("abc"),
        StringConstant("abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EmptyStringVersusEmptyString)
{
    LessThanExpression expression(
        StringConstant(""),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EmptyStringLessThanNonempty)
{
    LessThanExpression expression(
        StringConstant(""),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, NonemptyStringNotLessThanEmpty)
{
    LessThanExpression expression(
        StringConstant("Hello"),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, LongerStringCanBeSmaller)
{
    // "aaa" < "b"

    LessThanExpression expression(
        StringConstant("aaa"),
        StringConstant("b")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, ShorterStringNotAlwaysSmaller)
{
    // "z" < "aaaa" -> false

    LessThanExpression expression(
        StringConstant("z"),
        StringConstant("aaaa")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// UTF-8 bytewise comparisons
// --------------------------------------------------

TEST(LessThanExpressionTest, EqualUTF8Strings)
{
    LessThanExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, SmallerUTF8String)
{
    // UTF-8 è < é

    LessThanExpression expression(
        StringConstant("\xC3\xA8"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, GreaterUTF8String)
{
    // UTF-8 é > è

    LessThanExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA8")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, ASCIISmallerThanUTF8)
{
    LessThanExpression expression(
        StringConstant("e"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, UTF8GreaterThanASCII)
{
    LessThanExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("e")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, UTF8PrefixComparison)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, IntegerColumnLessThanConstant)
{
    LessThanExpression expression(
        Column(0),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, IntegerColumnGreaterThanConstant)
{
    LessThanExpression expression(
        Column(0),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, ConstantLessThanIntegerColumn)
{
    LessThanExpression expression(
        IntConstant(5),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, IntegerColumnLessThanIntegerColumn)
{
    LessThanExpression expression(
        Column(2),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, IntegerColumnGreaterThanIntegerColumn)
{
    LessThanExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EqualIntegerColumns)
{
    LessThanExpression expression(
        Column(0),
        Column(4)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, SameColumnComparedToItself)
{
    LessThanExpression expression(
        Column(0),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons - strings
// --------------------------------------------------

TEST(LessThanExpressionTest, StringColumnLessThanConstant)
{
    LessThanExpression expression(
        Column(1),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, StringColumnGreaterThanConstant)
{
    LessThanExpression expression(
        Column(5),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, StringColumnLessThanStringColumn)
{
    LessThanExpression expression(
        Column(1),
        Column(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, StringColumnGreaterThanStringColumn)
{
    LessThanExpression expression(
        Column(5),
        Column(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EqualStringColumns)
{
    LessThanExpression expression(
        Column(1),
        Column(7)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, EmptyStringColumnLessThanNonempty)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, AdditionLessThanConstant)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, SubtractionLessThanConstant)
{
    LessThanExpression expression(
        std::make_unique<SubtractionExpression>(
            IntConstant(20),
            IntConstant(5)
        ),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, AdditionLessThanSubtraction)
{
    // (5 + 5) < (20 - 5)

    LessThanExpression expression(
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

TEST(LessThanExpressionTest, AdditionGreaterThanSubtraction)
{
    // (10 + 10) < (20 - 5)

    LessThanExpression expression(
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

TEST(LessThanExpressionTest, EqualArithmeticResults)
{
    // (10 + 5) < (20 - 5)

    LessThanExpression expression(
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

TEST(LessThanExpressionTest, NestedArithmeticComparison)
{
    // ((20 - 5) + 10) < (30 - 10)

    LessThanExpression expression(
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

TEST(LessThanExpressionTest, LeftAdditionOverflow)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, RightSubtractionUnderflow)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, IntegerComparedWithString)
{
    LessThanExpression expression(
        IntConstant(10),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, StringComparedWithInteger)
{
    LessThanExpression expression(
        StringConstant("10"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, IntegerColumnComparedWithStringColumn)
{
    LessThanExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, StringColumnComparedWithIntegerConstant)
{
    LessThanExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, IntegerColumnComparedWithStringConstant)
{
    LessThanExpression expression(
        Column(0),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, BooleanComparedWithInteger)
{
    LessThanExpression expression(
        BoolExpression(true),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, IntegerComparedWithBoolean)
{
    LessThanExpression expression(
        IntConstant(1),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(LessThanExpressionTest, BooleanComparedWithString)
{
    LessThanExpression expression(
        BoolExpression(true),
        StringConstant("true")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

// --------------------------------------------------
// Invalid column positions
// --------------------------------------------------

TEST(LessThanExpressionTest, ColumnIndexOutOfBounds)
{
    LessThanExpression expression(
        Column(8),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(LessThanExpressionTest, VeryLargeColumnIndex)
{
    LessThanExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(LessThanExpressionTest, InvalidRightColumn)
{
    LessThanExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(LessThanExpressionTest, BothColumnsInvalid)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, IntegerEvaluationDoesNotModifyRow)
{
    LessThanExpression expression(
        Column(2),
        Column(0)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(LessThanExpressionTest, StringEvaluationDoesNotModifyRow)
{
    LessThanExpression expression(
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

TEST(LessThanExpressionTest, IntegerComparisonIsAsymmetric)
{
    LessThanExpression leftToRight(
        IntConstant(10),
        IntConstant(20)
    );

    LessThanExpression rightToLeft(
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

TEST(LessThanExpressionTest, StringComparisonIsAsymmetric)
{
    LessThanExpression leftToRight(
        StringConstant("Hello"),
        StringConstant("World")
    );

    LessThanExpression rightToLeft(
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

TEST(LessThanExpressionTest, EqualStringsAreNeverLess)
{
    LessThanExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(LessThanExpressionTest, MaximumLengthStringComparison)
{
    const std::string left(32, 'A');
    const std::string right(32, 'Z');

    LessThanExpression expression(
        StringConstant(left),
        StringConstant(right)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}
