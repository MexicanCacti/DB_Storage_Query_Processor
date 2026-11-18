
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
#include "not_equal_expression.hpp"
#include "greater_than_expression.hpp"
#include "greater_than_equal_expression.hpp"
#include "less_than_expression.hpp"
#include "less_than_equal_expression.hpp"
#include "or_expression.hpp"

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
// Boolean truth table
// --------------------------------------------------

TEST(OrExpressionTest, TrueOrTrue)
{
    OrExpression expression(
        BoolExpression(true),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TrueOrFalse)
{
    OrExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, FalseOrTrue)
{
    OrExpression expression(
        BoolExpression(false),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, FalseOrFalse)
{
    OrExpression expression(
        BoolExpression(false),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Comparison expressions
// --------------------------------------------------

TEST(OrExpressionTest, TwoTrueEqualExpressions)
{
    // (10 == 10) OR (20 == 20)

    OrExpression expression(
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

TEST(OrExpressionTest, TrueEqualOrFalseEqual)
{
    // (10 == 10) OR (20 == 30)

    OrExpression expression(
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

TEST(OrExpressionTest, FalseEqualOrTrueEqual)
{
    // (10 == 20) OR (30 == 30)

    OrExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<EqualExpression>(
            IntConstant(30),
            IntConstant(30)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TwoFalseEqualExpressions)
{
    // (10 == 20) OR (30 == 40)

    OrExpression expression(
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

TEST(OrExpressionTest, TwoTrueNotEqualExpressions)
{
    // (10 != 20) OR (30 != 40)

    OrExpression expression(
        std::make_unique<NotEqualExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<NotEqualExpression>(
            IntConstant(30),
            IntConstant(40)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TwoFalseNotEqualExpressions)
{
    // (10 != 10) OR (20 != 20)

    OrExpression expression(
        std::make_unique<NotEqualExpression>(
            IntConstant(10),
            IntConstant(10)
        ),
        std::make_unique<NotEqualExpression>(
            IntConstant(20),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(OrExpressionTest, EqualOrNotEqual)
{
    // (10 == 20) OR (20 != 30)

    OrExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<NotEqualExpression>(
            IntConstant(20),
            IntConstant(30)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, GreaterThanOrLessThan)
{
    // (20 > 10) OR (20 < 10)

    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            IntConstant(20),
            IntConstant(10)
        ),
        std::make_unique<LessThanExpression>(
            IntConstant(20),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, GreaterThanEqualOrLessThanEqual)
{
    // (10 >= 20) OR (20 <= 20)

    OrExpression expression(
        std::make_unique<GreaterThanEqualExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<LessThanEqualExpression>(
            IntConstant(20),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TwoFalseOrderingComparisons)
{
    // (10 > 20) OR (20 < 10)

    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            IntConstant(10),
            IntConstant(20)
        ),
        std::make_unique<LessThanExpression>(
            IntConstant(20),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String comparisons
// --------------------------------------------------

TEST(OrExpressionTest, TwoTrueStringComparisons)
{
    // ("Hello" == "Hello") OR ("Apple" < "World")

    OrExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant("Hello"),
            StringConstant("Hello")
        ),
        std::make_unique<LessThanExpression>(
            StringConstant("Apple"),
            StringConstant("World")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TrueOrFalseStringComparisons)
{
    // ("Hello" == "Hello") OR ("World" < "Apple")

    OrExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant("Hello"),
            StringConstant("Hello")
        ),
        std::make_unique<LessThanExpression>(
            StringConstant("World"),
            StringConstant("Apple")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, FalseOrTrueStringComparisons)
{
    // ("Hello" == "World") OR ("Apple" < "World")

    OrExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant("Hello"),
            StringConstant("World")
        ),
        std::make_unique<LessThanExpression>(
            StringConstant("Apple"),
            StringConstant("World")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TwoFalseStringComparisons)
{
    // ("Hello" == "World") OR ("World" < "Apple")

    OrExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant("Hello"),
            StringConstant("World")
        ),
        std::make_unique<LessThanExpression>(
            StringConstant("World"),
            StringConstant("Apple")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(OrExpressionTest, UTF8StringComparisons)
{
    // (è > é) OR (é == é)

    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            StringConstant("\xC3\xA8"),
            StringConstant("\xC3\xA9")
        ),
        std::make_unique<EqualExpression>(
            StringConstant("\xC3\xA9"),
            StringConstant("\xC3\xA9")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, EmptyStringComparisons)
{
    // ("" == "Hello") OR ("" < "Hello")

    OrExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant(""),
            StringConstant("Hello")
        ),
        std::make_unique<LessThanExpression>(
            StringConstant(""),
            StringConstant("Hello")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons
// --------------------------------------------------

TEST(OrExpressionTest, TwoTrueIntegerColumnComparisons)
{
    // (10 > 5) OR (-5 < 0)

    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            Column(0),
            IntConstant(5)
        ),
        std::make_unique<LessThanExpression>(
            Column(2),
            IntConstant(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TrueOrFalseIntegerColumnComparisons)
{
    // (10 > 5) OR (-5 > 0)

    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            Column(0),
            IntConstant(5)
        ),
        std::make_unique<GreaterThanExpression>(
            Column(2),
            IntConstant(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, FalseOrTrueIntegerColumnComparisons)
{
    // (10 < 5) OR (-5 < 0)

    OrExpression expression(
        std::make_unique<LessThanExpression>(
            Column(0),
            IntConstant(5)
        ),
        std::make_unique<LessThanExpression>(
            Column(2),
            IntConstant(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TwoFalseIntegerColumnComparisons)
{
    // (10 < 5) OR (-5 > 0)

    OrExpression expression(
        std::make_unique<LessThanExpression>(
            Column(0),
            IntConstant(5)
        ),
        std::make_unique<GreaterThanExpression>(
            Column(2),
            IntConstant(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(OrExpressionTest, EqualIntegerColumns)
{
    // (Column 0 == Column 4) OR (Column 2 > Column 0)

    OrExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            Column(4)
        ),
        std::make_unique<GreaterThanExpression>(
            Column(2),
            Column(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, StringColumnComparisons)
{
    // (Column 1 != Column 7) OR (Column 1 < Column 5)

    OrExpression expression(
        std::make_unique<NotEqualExpression>(
            Column(1),
            Column(7)
        ),
        std::make_unique<LessThanExpression>(
            Column(1),
            Column(5)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, IntegerAndStringColumnPredicates)
{
    // (Column 0 == 20) OR (Column 1 == "Hello")

    OrExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            IntConstant(20)
        ),
        std::make_unique<EqualExpression>(
            Column(1),
            StringConstant("Hello")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested OR expressions
// --------------------------------------------------

TEST(OrExpressionTest, NestedOrAllTrue)
{
    // (true OR true) OR true

    OrExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(true),
            BoolExpression(true)
        ),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, NestedOrWithOneTrue)
{
    // (false OR false) OR true

    OrExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(false),
            BoolExpression(false)
        ),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, NestedOrAllFalse)
{
    // (false OR false) OR false

    OrExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(false),
            BoolExpression(false)
        ),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(OrExpressionTest, RightNestedOrAllTrue)
{
    // true OR (true OR true)

    OrExpression expression(
        BoolExpression(true),
        std::make_unique<OrExpression>(
            BoolExpression(true),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, RightNestedOrWithOneTrue)
{
    // false OR (false OR true)

    OrExpression expression(
        BoolExpression(false),
        std::make_unique<OrExpression>(
            BoolExpression(false),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, DeeplyNestedOr)
{
    // ((false OR false) OR true) OR false

    OrExpression expression(
        std::make_unique<OrExpression>(
            std::make_unique<OrExpression>(
                BoolExpression(false),
                BoolExpression(false)
            ),
            BoolExpression(true)
        ),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, DeeplyNestedOrAllFalse)
{
    OrExpression expression(
        std::make_unique<OrExpression>(
            std::make_unique<OrExpression>(
                BoolExpression(false),
                BoolExpression(false)
            ),
            BoolExpression(false)
        ),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested arithmetic comparisons
// --------------------------------------------------

TEST(OrExpressionTest, AdditionOrSubtractionPredicates)
{
    // (10 + 5 > 20) OR (20 - 5 == 15)

    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(10),
                IntConstant(5)
            ),
            IntConstant(20)
        ),
        std::make_unique<EqualExpression>(
            std::make_unique<SubtractionExpression>(
                IntConstant(20),
                IntConstant(5)
            ),
            IntConstant(15)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, ColumnArithmeticPredicates)
{
    // (Column 0 + Column 2 == 100)
    // OR (Column 0 - Column 2 == 15)

    OrExpression expression(
        std::make_unique<EqualExpression>(
            std::make_unique<AdditionExpression>(
                Column(0),
                Column(2)
            ),
            IntConstant(100)
        ),
        std::make_unique<EqualExpression>(
            std::make_unique<SubtractionExpression>(
                Column(0),
                Column(2)
            ),
            IntConstant(15)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, BothArithmeticPredicatesFalse)
{
    OrExpression expression(
        std::make_unique<EqualExpression>(
            std::make_unique<AdditionExpression>(
                Column(0),
                Column(2)
            ),
            IntConstant(100)
        ),
        std::make_unique<EqualExpression>(
            std::make_unique<SubtractionExpression>(
                Column(0),
                Column(2)
            ),
            IntConstant(100)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(OrExpressionTest, IntegerOrIntegerRejected)
{
    OrExpression expression(
        IntConstant(1),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, ZeroOrOneRejected)
{
    OrExpression expression(
        IntConstant(0),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, StringOrStringRejected)
{
    OrExpression expression(
        StringConstant("true"),
        StringConstant("false")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, IntegerOrBooleanRejected)
{
    OrExpression expression(
        IntConstant(1),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, BooleanOrIntegerRejected)
{
    OrExpression expression(
        BoolExpression(false),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, StringOrBooleanRejected)
{
    OrExpression expression(
        StringConstant("true"),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, BooleanOrStringRejected)
{
    OrExpression expression(
        BoolExpression(false),
        StringConstant("true")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, IntegerColumnOrBooleanRejected)
{
    OrExpression expression(
        Column(0),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, BooleanOrStringColumnRejected)
{
    OrExpression expression(
        BoolExpression(false),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(OrExpressionTest, ArithmeticResultOrBooleanRejected)
{
    OrExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(5)
        ),
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

TEST(OrExpressionTest, InvalidLeftColumn)
{
    OrExpression expression(
        std::make_unique<EqualExpression>(
            Column(8),
            IntConstant(10)
        ),
        BoolExpression(false)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(OrExpressionTest, InvalidRightColumn)
{
    OrExpression expression(
        BoolExpression(false),
        std::make_unique<EqualExpression>(
            Column(100),
            IntConstant(10)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(OrExpressionTest, VeryLargeColumnIndex)
{
    OrExpression expression(
        std::make_unique<EqualExpression>(
            Column(std::numeric_limits<std::size_t>::max()),
            IntConstant(10)
        ),
        BoolExpression(false)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Overflow propagation
// --------------------------------------------------

TEST(OrExpressionTest, LeftAdditionOverflow)
{
    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(std::numeric_limits<int64_t>::max()),
                IntConstant(1)
            ),
            IntConstant(0)
        ),
        BoolExpression(false)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(OrExpressionTest, RightSubtractionUnderflow)
{
    OrExpression expression(
        BoolExpression(false),
        std::make_unique<LessThanExpression>(
            std::make_unique<SubtractionExpression>(
                IntConstant(std::numeric_limits<int64_t>::min()),
                IntConstant(1)
            ),
            IntConstant(0)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

// --------------------------------------------------
// Short-circuit evaluation
// --------------------------------------------------

// These tests assume conventional short-circuit OR:
// when the left operand is true, the right operand
// must not be evaluated.

TEST(OrExpressionTest, TrueLeftSkipsInvalidRightColumn)
{
    OrExpression expression(
        BoolExpression(true),
        std::make_unique<EqualExpression>(
            Column(100),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TrueLeftSkipsRightOverflow)
{
    OrExpression expression(
        BoolExpression(true),
        std::make_unique<GreaterThanExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(std::numeric_limits<int64_t>::max()),
                IntConstant(1)
            ),
            IntConstant(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, FalseLeftEvaluatesInvalidRightColumn)
{
    OrExpression expression(
        BoolExpression(false),
        std::make_unique<EqualExpression>(
            Column(100),
            IntConstant(10)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(OrExpressionTest, FalseLeftEvaluatesRightOverflow)
{
    OrExpression expression(
        BoolExpression(false),
        std::make_unique<GreaterThanExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(std::numeric_limits<int64_t>::max()),
                IntConstant(1)
            ),
            IntConstant(0)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

// --------------------------------------------------
// Input row immutability
// --------------------------------------------------

TEST(OrExpressionTest, IntegerPredicatesDoNotModifyRow)
{
    OrExpression expression(
        std::make_unique<GreaterThanExpression>(
            Column(0),
            IntConstant(20)
        ),
        std::make_unique<LessThanExpression>(
            Column(2),
            IntConstant(0)
        )
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(OrExpressionTest, StringPredicatesDoNotModifyRow)
{
    OrExpression expression(
        std::make_unique<NotEqualExpression>(
            Column(1),
            Column(7)
        ),
        std::make_unique<LessThanExpression>(
            Column(1),
            Column(5)
        )
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

// --------------------------------------------------
// Logical properties
// --------------------------------------------------

TEST(OrExpressionTest, CommutativeProperty)
{
    OrExpression leftToRight(
        BoolExpression(true),
        BoolExpression(false)
    );

    OrExpression rightToLeft(
        BoolExpression(false),
        BoolExpression(true)
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

TEST(OrExpressionTest, AssociativeProperty)
{
    // (false OR true) OR false
    // false OR (true OR false)

    OrExpression leftAssociative(
        std::make_unique<OrExpression>(
            BoolExpression(false),
            BoolExpression(true)
        ),
        BoolExpression(false)
    );

    OrExpression rightAssociative(
        BoolExpression(false),
        std::make_unique<OrExpression>(
            BoolExpression(true),
            BoolExpression(false)
        )
    );

    ExpressionVariant result1 = leftAssociative.Eval(testRow);
    ExpressionVariant result2 = rightAssociative.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result1));
    ASSERT_TRUE(std::holds_alternative<bool>(result2));

    EXPECT_EQ(
        std::get<bool>(result1),
        std::get<bool>(result2)
    );

    EXPECT_TRUE(std::get<bool>(result1));
}

TEST(OrExpressionTest, FalseIdentity)
{
    // false OR x = x

    OrExpression expression(
        BoolExpression(false),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, TrueAnnihilator)
{
    // true OR x = true

    OrExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(OrExpressionTest, IdempotentProperty)
{
    // x OR x = x

    OrExpression expression(
        BoolExpression(false),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Boolean return type
// --------------------------------------------------

TEST(OrExpressionTest, TrueResultIsBoolean)
{
    OrExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(OrExpressionTest, FalseResultIsBoolean)
{
    OrExpression expression(
        BoolExpression(false),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}
