
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
#include "and_expression.hpp"

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

TEST(AndExpressionTest, TrueAndTrue)
{
    AndExpression expression(
        BoolExpression(true),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(AndExpressionTest, TrueAndFalse)
{
    AndExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, FalseAndTrue)
{
    AndExpression expression(
        BoolExpression(false),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, FalseAndFalse)
{
    AndExpression expression(
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

TEST(AndExpressionTest, TwoTrueEqualExpressions)
{
    // (10 == 10) AND (20 == 20)

    AndExpression expression(
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

TEST(AndExpressionTest, TrueEqualAndFalseEqual)
{
    // (10 == 10) AND (20 == 30)

    AndExpression expression(
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

TEST(AndExpressionTest, TwoTrueNotEqualExpressions)
{
    // (10 != 20) AND (30 != 40)

    AndExpression expression(
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

TEST(AndExpressionTest, EqualAndNotEqual)
{
    // (10 == 10) AND (20 != 30)

    AndExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
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

TEST(AndExpressionTest, GreaterThanAndLessThan)
{
    // (20 > 10) AND (5 < 10)

    AndExpression expression(
        std::make_unique<GreaterThanExpression>(
            IntConstant(20),
            IntConstant(10)
        ),
        std::make_unique<LessThanExpression>(
            IntConstant(5),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(AndExpressionTest, GreaterThanEqualAndLessThanEqual)
{
    // (10 >= 10) AND (20 <= 20)

    AndExpression expression(
        std::make_unique<GreaterThanEqualExpression>(
            IntConstant(10),
            IntConstant(10)
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

TEST(AndExpressionTest, TrueGreaterThanAndFalseLessThan)
{
    // (20 > 10) AND (20 < 10)

    AndExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// String comparisons
// --------------------------------------------------

TEST(AndExpressionTest, TwoTrueStringComparisons)
{
    // ("Hello" == "Hello") AND ("Apple" < "World")

    AndExpression expression(
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

TEST(AndExpressionTest, TrueAndFalseStringComparisons)
{
    // ("Hello" == "Hello") AND ("World" < "Apple")

    AndExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, UTF8StringComparisons)
{
    // (é > è) AND (é == é)

    AndExpression expression(
        std::make_unique<GreaterThanExpression>(
            StringConstant("\xC3\xA9"),
            StringConstant("\xC3\xA8")
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

TEST(AndExpressionTest, EmptyStringComparisons)
{
    // ("" == "") AND ("" < "Hello")

    AndExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant(""),
            StringConstant("")
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

TEST(AndExpressionTest, TwoTrueIntegerColumnComparisons)
{
    // (Column 0 > 5) AND (Column 2 < 0)
    // (10 > 5) AND (-5 < 0)

    AndExpression expression(
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

TEST(AndExpressionTest, TrueAndFalseIntegerColumnComparisons)
{
    // (Column 0 > 5) AND (Column 2 > 0)

    AndExpression expression(
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, EqualIntegerColumns)
{
    // (Column 0 == Column 4) AND (Column 2 < Column 0)

    AndExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            Column(4)
        ),
        std::make_unique<LessThanExpression>(
            Column(2),
            Column(0)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(AndExpressionTest, StringColumnComparisons)
{
    // (Column 1 == Column 7) AND (Column 1 < Column 5)

    AndExpression expression(
        std::make_unique<EqualExpression>(
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

TEST(AndExpressionTest, IntegerAndStringColumnPredicates)
{
    // (Column 0 == 10) AND (Column 1 == "Hello")

    AndExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            IntConstant(10)
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
// Nested AND expressions
// --------------------------------------------------

TEST(AndExpressionTest, NestedAndAllTrue)
{
    // (true AND true) AND true

    AndExpression expression(
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(true)
        ),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(AndExpressionTest, NestedAndWithFalse)
{
    // (true AND false) AND true

    AndExpression expression(
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(false)
        ),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, RightNestedAndAllTrue)
{
    // true AND (true AND true)

    AndExpression expression(
        BoolExpression(true),
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(AndExpressionTest, RightNestedAndWithFalse)
{
    // true AND (true AND false)

    AndExpression expression(
        BoolExpression(true),
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, DeeplyNestedAnd)
{
    // ((true AND true) AND true) AND true

    AndExpression expression(
        std::make_unique<AndExpression>(
            std::make_unique<AndExpression>(
                BoolExpression(true),
                BoolExpression(true)
            ),
            BoolExpression(true)
        ),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(AndExpressionTest, DeeplyNestedAndWithFalse)
{
    // ((true AND true) AND false) AND true

    AndExpression expression(
        std::make_unique<AndExpression>(
            std::make_unique<AndExpression>(
                BoolExpression(true),
                BoolExpression(true)
            ),
            BoolExpression(false)
        ),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested arithmetic comparisons
// --------------------------------------------------

TEST(AndExpressionTest, AdditionAndSubtractionPredicates)
{
    // (10 + 5 > 10) AND (20 - 5 == 15)

    AndExpression expression(
        std::make_unique<GreaterThanExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(10),
                IntConstant(5)
            ),
            IntConstant(10)
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

TEST(AndExpressionTest, ColumnArithmeticPredicates)
{
    // (Column 0 + Column 2 == 5)
    // AND (Column 0 - Column 2 == 15)

    AndExpression expression(
        std::make_unique<EqualExpression>(
            std::make_unique<AdditionExpression>(
                Column(0),
                Column(2)
            ),
            IntConstant(5)
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

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(AndExpressionTest, IntegerAndIntegerRejected)
{
    AndExpression expression(
        IntConstant(1),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, ZeroAndOneRejected)
{
    AndExpression expression(
        IntConstant(0),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, StringAndStringRejected)
{
    AndExpression expression(
        StringConstant("true"),
        StringConstant("false")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, IntegerAndBooleanRejected)
{
    AndExpression expression(
        IntConstant(1),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, BooleanAndIntegerRejected)
{
    AndExpression expression(
        BoolExpression(true),
        IntConstant(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, StringAndBooleanRejected)
{
    AndExpression expression(
        StringConstant("true"),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, BooleanAndStringRejected)
{
    AndExpression expression(
        BoolExpression(true),
        StringConstant("true")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, IntegerColumnAndBooleanRejected)
{
    AndExpression expression(
        Column(0),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, BooleanAndStringColumnRejected)
{
    AndExpression expression(
        BoolExpression(true),
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(AndExpressionTest, ArithmeticResultAndBooleanRejected)
{
    AndExpression expression(
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

TEST(AndExpressionTest, InvalidLeftColumn)
{
    AndExpression expression(
        std::make_unique<EqualExpression>(
            Column(8),
            IntConstant(10)
        ),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(AndExpressionTest, InvalidRightColumn)
{
    AndExpression expression(
        BoolExpression(true),
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

TEST(AndExpressionTest, VeryLargeColumnIndex)
{
    AndExpression expression(
        std::make_unique<EqualExpression>(
            Column(std::numeric_limits<std::size_t>::max()),
            IntConstant(10)
        ),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Overflow propagation
// --------------------------------------------------

TEST(AndExpressionTest, LeftAdditionOverflow)
{
    AndExpression expression(
        std::make_unique<GreaterThanExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(std::numeric_limits<int64_t>::max()),
                IntConstant(1)
            ),
            IntConstant(0)
        ),
        BoolExpression(true)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::overflow_error
    );
}

TEST(AndExpressionTest, RightSubtractionUnderflow)
{
    AndExpression expression(
        BoolExpression(true),
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

// These tests assume logical AND uses short-circuit
// evaluation. If the left operand evaluates to false,
// the right operand must not be evaluated.

TEST(AndExpressionTest, FalseLeftSkipsInvalidRightColumn)
{
    AndExpression expression(
        BoolExpression(false),
        std::make_unique<EqualExpression>(
            Column(100),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, FalseLeftSkipsRightOverflow)
{
    AndExpression expression(
        BoolExpression(false),
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
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, TrueLeftEvaluatesInvalidRightColumn)
{
    AndExpression expression(
        BoolExpression(true),
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

TEST(AndExpressionTest, TrueLeftEvaluatesRightOverflow)
{
    AndExpression expression(
        BoolExpression(true),
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

TEST(AndExpressionTest, IntegerPredicatesDoNotModifyRow)
{
    AndExpression expression(
        std::make_unique<GreaterThanExpression>(
            Column(0),
            IntConstant(5)
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

TEST(AndExpressionTest, StringPredicatesDoNotModifyRow)
{
    AndExpression expression(
        std::make_unique<EqualExpression>(
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

TEST(AndExpressionTest, CommutativeProperty)
{
    AndExpression leftToRight(
        BoolExpression(true),
        BoolExpression(false)
    );

    AndExpression rightToLeft(
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
}

TEST(AndExpressionTest, AssociativeProperty)
{
    // (true AND false) AND true
    // true AND (false AND true)

    AndExpression leftAssociative(
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(false)
        ),
        BoolExpression(true)
    );

    AndExpression rightAssociative(
        BoolExpression(true),
        std::make_unique<AndExpression>(
            BoolExpression(false),
            BoolExpression(true)
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
}

TEST(AndExpressionTest, TrueIdentity)
{
    // true AND x = x

    AndExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, FalseAnnihilator)
{
    // false AND x = false

    AndExpression expression(
        BoolExpression(false),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(AndExpressionTest, IdempotentProperty)
{
    // x AND x = x

    AndExpression expression(
        BoolExpression(true),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Boolean return type
// --------------------------------------------------

TEST(AndExpressionTest, TrueResultIsBoolean)
{
    AndExpression expression(
        BoolExpression(true),
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(AndExpressionTest, FalseResultIsBoolean)
{
    AndExpression expression(
        BoolExpression(true),
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}
