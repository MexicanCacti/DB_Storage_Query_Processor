
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
#include "greater_than_expression.hpp"

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
    if(value)
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

TEST(GreaterThanExpressionTest, GreaterPositiveIntegers)
{
    GreaterThanExpression expression(
        IntConstant(20),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, SmallerPositiveIntegers)
{
    GreaterThanExpression expression(
        IntConstant(10),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EqualPositiveIntegers)
{
    GreaterThanExpression expression(
        IntConstant(10),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, GreaterNegativeIntegers)
{
    GreaterThanExpression expression(
        IntConstant(-5),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, SmallerNegativeIntegers)
{
    GreaterThanExpression expression(
        IntConstant(-20),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EqualNegativeIntegers)
{
    GreaterThanExpression expression(
        IntConstant(-10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, PositiveGreaterThanNegative)
{
    GreaterThanExpression expression(
        IntConstant(10),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, NegativeGreaterThanPositive)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, PositiveGreaterThanZero)
{
    GreaterThanExpression expression(
        IntConstant(10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ZeroGreaterThanPositive)
{
    GreaterThanExpression expression(
        IntConstant(0),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ZeroGreaterThanNegative)
{
    GreaterThanExpression expression(
        IntConstant(0),
        IntConstant(-10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, NegativeGreaterThanZero)
{
    GreaterThanExpression expression(
        IntConstant(-10),
        IntConstant(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ZeroGreaterThanZero)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, MaximumGreaterThanMinimum)
{
    GreaterThanExpression expression(
        IntConstant(std::numeric_limits<int64_t>::max()),
        IntConstant(std::numeric_limits<int64_t>::min())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, MinimumGreaterThanMaximum)
{
    GreaterThanExpression expression(
        IntConstant(std::numeric_limits<int64_t>::min()),
        IntConstant(std::numeric_limits<int64_t>::max())
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, MaximumGreaterThanMaximum)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    GreaterThanExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, MinimumGreaterThanMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    GreaterThanExpression expression(
        IntConstant(minValue),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, MaximumGreaterThanMaximumMinusOne)
{
    const int64_t maxValue =
        std::numeric_limits<int64_t>::max();

    GreaterThanExpression expression(
        IntConstant(maxValue),
        IntConstant(maxValue - 1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, MinimumPlusOneGreaterThanMinimum)
{
    const int64_t minValue =
        std::numeric_limits<int64_t>::min();

    GreaterThanExpression expression(
        IntConstant(minValue + 1),
        IntConstant(minValue)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// String comparisons - ASCII
// --------------------------------------------------

TEST(GreaterThanExpressionTest, LexicographicallyGreaterString)
{
    GreaterThanExpression expression(
        StringConstant("World"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, LexicographicallySmallerString)
{
    GreaterThanExpression expression(
        StringConstant("Apple"),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EqualStrings)
{
    GreaterThanExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, FirstCharacterDeterminesOrder)
{
    GreaterThanExpression expression(
        StringConstant("Zebra"),
        StringConstant("Apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, LaterCharacterDeterminesOrder)
{
    // "abd" > "abc"

    GreaterThanExpression expression(
        StringConstant("abd"),
        StringConstant("abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EarlierCharacterTakesPrecedence)
{
    // 'a' < 'z', so "az" < "za"

    GreaterThanExpression expression(
        StringConstant("az"),
        StringConstant("za")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, LaterGreaterCharacterCannotOverrideEarlierSmaller)
{
    // "abz" < "aca" because 'b' < 'c'

    GreaterThanExpression expression(
        StringConstant("abz"),
        StringConstant("aca")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, FirstDifferingCharacterDeterminesOrder)
{
    // "abz" > "aby"

    GreaterThanExpression expression(
        StringConstant("abz"),
        StringConstant("aby")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, UppercaseVersusLowercase)
{
    // ASCII 'a' > 'A'

    GreaterThanExpression expression(
        StringConstant("apple"),
        StringConstant("Apple")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, LowercaseComparedWithUppercase)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, LongerStringWithEqualPrefix)
{
    GreaterThanExpression expression(
        StringConstant("HelloWorld"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ShorterStringWithEqualPrefix)
{
    GreaterThanExpression expression(
        StringConstant("Hello"),
        StringConstant("HelloWorld")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EqualLengthStrings)
{
    GreaterThanExpression expression(
        StringConstant("abc"),
        StringConstant("abc")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EmptyStringVersusEmptyString)
{
    GreaterThanExpression expression(
        StringConstant(""),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, NonemptyStringGreaterThanEmpty)
{
    GreaterThanExpression expression(
        StringConstant("Hello"),
        StringConstant("")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EmptyStringNotGreaterThanNonempty)
{
    GreaterThanExpression expression(
        StringConstant(""),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, LongerStringNotAlwaysGreater)
{
    // "aaa" < "b"

    GreaterThanExpression expression(
        StringConstant("aaa"),
        StringConstant("b")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ShorterStringCanBeGreater)
{
    // "z" > "aaaa"

    GreaterThanExpression expression(
        StringConstant("z"),
        StringConstant("aaaa")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// UTF-8 bytewise comparisons
// --------------------------------------------------

TEST(GreaterThanExpressionTest, EqualUTF8Strings)
{
    GreaterThanExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, GreaterUTF8String)
{
    // UTF-8 é > è

    GreaterThanExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("\xC3\xA8")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, SmallerUTF8String)
{
    // UTF-8 è < é

    GreaterThanExpression expression(
        StringConstant("\xC3\xA8"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, UTF8GreaterThanASCII)
{
    GreaterThanExpression expression(
        StringConstant("\xC3\xA9"),
        StringConstant("e")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ASCIISmallerThanUTF8)
{
    GreaterThanExpression expression(
        StringConstant("e"),
        StringConstant("\xC3\xA9")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, UTF8PrefixComparison)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, IntegerColumnGreaterThanConstant)
{
    GreaterThanExpression expression(
        Column(0),
        IntConstant(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, IntegerColumnSmallerThanConstant)
{
    GreaterThanExpression expression(
        Column(0),
        IntConstant(20)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, ConstantGreaterThanIntegerColumn)
{
    GreaterThanExpression expression(
        IntConstant(20),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, IntegerColumnGreaterThanIntegerColumn)
{
    GreaterThanExpression expression(
        Column(0),
        Column(2)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, IntegerColumnSmallerThanIntegerColumn)
{
    GreaterThanExpression expression(
        Column(2),
        Column(0)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EqualIntegerColumns)
{
    GreaterThanExpression expression(
        Column(0),
        Column(4)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, SameColumnComparedToItself)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, StringColumnGreaterThanConstant)
{
    GreaterThanExpression expression(
        Column(5),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, StringColumnSmallerThanConstant)
{
    GreaterThanExpression expression(
        Column(1),
        StringConstant("World")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, StringColumnGreaterThanStringColumn)
{
    GreaterThanExpression expression(
        Column(5),
        Column(1)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, StringColumnSmallerThanStringColumn)
{
    GreaterThanExpression expression(
        Column(1),
        Column(5)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, EqualStringColumns)
{
    GreaterThanExpression expression(
        Column(1),
        Column(7)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, NonemptyStringColumnGreaterThanEmpty)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, AdditionGreaterThanConstant)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, SubtractionGreaterThanConstant)
{
    GreaterThanExpression expression(
        std::make_unique<SubtractionExpression>(
            IntConstant(20),
            IntConstant(5)
        ),
        IntConstant(10)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, AdditionGreaterThanSubtraction)
{
    // (10 + 10) > (20 - 5)

    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, AdditionSmallerThanSubtraction)
{
    // (5 + 5) > (20 - 5)

    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, EqualArithmeticResults)
{
    // (10 + 5) > (20 - 5)

    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, NestedArithmeticComparison)
{
    // ((20 - 5) + 10) > (30 - 10)

    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, LeftAdditionOverflow)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, RightSubtractionUnderflow)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, IntegerComparedWithString)
{
    GreaterThanExpression expression(
        IntConstant(10),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, StringComparedWithInteger)
{
    GreaterThanExpression expression(
        StringConstant("10"),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, IntegerColumnComparedWithStringColumn)
{
    GreaterThanExpression expression(
        Column(0),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, StringColumnComparedWithIntegerConstant)
{
    GreaterThanExpression expression(
        Column(1),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, IntegerColumnComparedWithStringConstant)
{
    GreaterThanExpression expression(
        Column(0),
        StringConstant("10")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, BooleanComparedWithInteger)
{
    GreaterThanExpression expression(
        BoolExpression(true),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, IntegerComparedWithBoolean)
{
    GreaterThanExpression expression(
        IntConstant(1),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(GreaterThanExpressionTest, BooleanComparedWithString)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, ColumnIndexOutOfBounds)
{
    GreaterThanExpression expression(
        Column(8),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(GreaterThanExpressionTest, VeryLargeColumnIndex)
{
    GreaterThanExpression expression(
        Column(std::numeric_limits<std::size_t>::max()),
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(GreaterThanExpressionTest, InvalidRightColumn)
{
    GreaterThanExpression expression(
        IntConstant(10),
        Column(100)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(GreaterThanExpressionTest, BothColumnsInvalid)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, IntegerEvaluationDoesNotModifyRow)
{
    GreaterThanExpression expression(
        Column(0),
        Column(2)
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(GreaterThanExpressionTest, StringEvaluationDoesNotModifyRow)
{
    GreaterThanExpression expression(
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

TEST(GreaterThanExpressionTest, IntegerComparisonIsAsymmetric)
{
    GreaterThanExpression leftToRight(
        IntConstant(20),
        IntConstant(10)
    );

    GreaterThanExpression rightToLeft(
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

TEST(GreaterThanExpressionTest, StringComparisonIsAsymmetric)
{
    GreaterThanExpression leftToRight(
        StringConstant("World"),
        StringConstant("Hello")
    );

    GreaterThanExpression rightToLeft(
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

TEST(GreaterThanExpressionTest, EqualStringsAreNeverGreater)
{
    GreaterThanExpression expression(
        StringConstant("Hello"),
        StringConstant("Hello")
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(GreaterThanExpressionTest, MaximumLengthStringComparison)
{
    const std::string left(32, 'Z');
    const std::string right(32, 'A');

    GreaterThanExpression expression(
        StringConstant(left),
        StringConstant(right)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}
