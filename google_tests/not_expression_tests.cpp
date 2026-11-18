
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
#include "or_expression.hpp"
#include "not_expression.hpp"

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
// Basic boolean negation
// --------------------------------------------------

TEST(NotExpressionTest, NotTrue)
{
    NotExpression expression(
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalse)
{
    NotExpression expression(
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Negation of equality comparisons
// --------------------------------------------------

TEST(NotExpressionTest, NotEqualIntegers)
{
    // NOT (10 == 10) -> false

    NotExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotUnequalIntegers)
{
    // NOT (10 == 20) -> true

    NotExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotEqualStrings)
{
    NotExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant("Hello"),
            StringConstant("Hello")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotUnequalStrings)
{
    NotExpression expression(
        std::make_unique<EqualExpression>(
            StringConstant("Hello"),
            StringConstant("World")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Negation of inequality comparisons
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueNotEqual)
{
    // NOT (10 != 20) -> false

    NotExpression expression(
        std::make_unique<NotEqualExpression>(
            IntConstant(10),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseNotEqual)
{
    // NOT (10 != 10) -> true

    NotExpression expression(
        std::make_unique<NotEqualExpression>(
            IntConstant(10),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Negation of ordering comparisons
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueGreaterThan)
{
    // NOT (20 > 10) -> false

    NotExpression expression(
        std::make_unique<GreaterThanExpression>(
            IntConstant(20),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseGreaterThan)
{
    // NOT (10 > 20) -> true

    NotExpression expression(
        std::make_unique<GreaterThanExpression>(
            IntConstant(10),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueLessThan)
{
    // NOT (10 < 20) -> false

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            IntConstant(10),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseLessThan)
{
    // NOT (20 < 10) -> true

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            IntConstant(20),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueGreaterThanEqual)
{
    NotExpression expression(
        std::make_unique<GreaterThanEqualExpression>(
            IntConstant(10),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseGreaterThanEqual)
{
    NotExpression expression(
        std::make_unique<GreaterThanEqualExpression>(
            IntConstant(10),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueLessThanEqual)
{
    NotExpression expression(
        std::make_unique<LessThanEqualExpression>(
            IntConstant(10),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseLessThanEqual)
{
    NotExpression expression(
        std::make_unique<LessThanEqualExpression>(
            IntConstant(20),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// String ordering comparisons
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueStringLessThan)
{
    // NOT ("Apple" < "World") -> false

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            StringConstant("Apple"),
            StringConstant("World")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseStringLessThan)
{
    // NOT ("World" < "Apple") -> true

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            StringConstant("World"),
            StringConstant("Apple")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueUTF8Comparison)
{
    // NOT (è < é) -> false

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            StringConstant("\xC3\xA8"),
            StringConstant("\xC3\xA9")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseUTF8Comparison)
{
    // NOT (é < è) -> true

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            StringConstant("\xC3\xA9"),
            StringConstant("\xC3\xA8")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested NOT expressions
// --------------------------------------------------

TEST(NotExpressionTest, DoubleNegationTrue)
{
    // NOT (NOT true) -> true

    NotExpression expression(
        std::make_unique<NotExpression>(
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, DoubleNegationFalse)
{
    // NOT (NOT false) -> false

    NotExpression expression(
        std::make_unique<NotExpression>(
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, TripleNegationTrue)
{
    // NOT (NOT (NOT true)) -> false

    NotExpression expression(
        std::make_unique<NotExpression>(
            std::make_unique<NotExpression>(
                BoolExpression(true)
            )
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, TripleNegationFalse)
{
    // NOT (NOT (NOT false)) -> true

    NotExpression expression(
        std::make_unique<NotExpression>(
            std::make_unique<NotExpression>(
                BoolExpression(false)
            )
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Negation of AND expressions
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueAndTrue)
{
    // NOT (true AND true) -> false

    NotExpression expression(
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueAndFalse)
{
    // NOT (true AND false) -> true

    NotExpression expression(
        std::make_unique<AndExpression>(
            BoolExpression(true),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseAndTrue)
{
    // NOT (false AND true) -> true

    NotExpression expression(
        std::make_unique<AndExpression>(
            BoolExpression(false),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseAndFalse)
{
    // NOT (false AND false) -> true

    NotExpression expression(
        std::make_unique<AndExpression>(
            BoolExpression(false),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Negation of OR expressions
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueOrTrue)
{
    // NOT (true OR true) -> false

    NotExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(true),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueOrFalse)
{
    // NOT (true OR false) -> false

    NotExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(true),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseOrTrue)
{
    // NOT (false OR true) -> false

    NotExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(false),
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseOrFalse)
{
    // NOT (false OR false) -> true

    NotExpression expression(
        std::make_unique<OrExpression>(
            BoolExpression(false),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Column comparisons
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueIntegerColumnComparison)
{
    // NOT (Column 0 == 10) -> false

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseIntegerColumnComparison)
{
    // NOT (Column 0 == 20) -> true

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueStringColumnComparison)
{
    // NOT (Column 1 == "Hello") -> false

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(1),
            StringConstant("Hello")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseStringColumnComparison)
{
    // NOT (Column 1 == "World") -> true

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(1),
            StringConstant("World")
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotEqualIntegerColumns)
{
    // NOT (Column 0 == Column 4) -> false

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            Column(4)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotUnequalIntegerColumns)
{
    // NOT (Column 0 == Column 2) -> true

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(0),
            Column(2)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotEqualStringColumns)
{
    // NOT (Column 1 == Column 7) -> false

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(1),
            Column(7)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotUnequalStringColumns)
{
    // NOT (Column 1 == Column 5) -> true

    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(1),
            Column(5)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Nested arithmetic comparisons
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueAdditionComparison)
{
    // NOT ((10 + 5) == 15) -> false

    NotExpression expression(
        std::make_unique<EqualExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(10),
                IntConstant(5)
            ),
            IntConstant(15)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseAdditionComparison)
{
    // NOT ((10 + 5) == 20) -> true

    NotExpression expression(
        std::make_unique<EqualExpression>(
            std::make_unique<AdditionExpression>(
                IntConstant(10),
                IntConstant(5)
            ),
            IntConstant(20)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotTrueSubtractionComparison)
{
    // NOT ((20 - 5) > 10) -> false

    NotExpression expression(
        std::make_unique<GreaterThanExpression>(
            std::make_unique<SubtractionExpression>(
                IntConstant(20),
                IntConstant(5)
            ),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotFalseSubtractionComparison)
{
    // NOT ((20 - 5) < 10) -> true

    NotExpression expression(
        std::make_unique<LessThanExpression>(
            std::make_unique<SubtractionExpression>(
                IntConstant(20),
                IntConstant(5)
            ),
            IntConstant(10)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

// --------------------------------------------------
// Type mismatch rejection
// --------------------------------------------------

TEST(NotExpressionTest, PositiveIntegerRejected)
{
    NotExpression expression(
        IntConstant(10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, NegativeIntegerRejected)
{
    NotExpression expression(
        IntConstant(-10)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, ZeroIntegerRejected)
{
    NotExpression expression(
        IntConstant(0)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, NonemptyStringRejected)
{
    NotExpression expression(
        StringConstant("Hello")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, EmptyStringRejected)
{
    NotExpression expression(
        StringConstant("")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, UTF8StringRejected)
{
    NotExpression expression(
        StringConstant("\xC3\xA9")
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, IntegerColumnRejected)
{
    NotExpression expression(
        Column(0)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, StringColumnRejected)
{
    NotExpression expression(
        Column(1)
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, AdditionResultRejected)
{
    NotExpression expression(
        std::make_unique<AdditionExpression>(
            IntConstant(10),
            IntConstant(5)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

TEST(NotExpressionTest, SubtractionResultRejected)
{
    NotExpression expression(
        std::make_unique<SubtractionExpression>(
            IntConstant(20),
            IntConstant(5)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::invalid_argument
    );
}

// --------------------------------------------------
// Invalid column positions
// --------------------------------------------------

TEST(NotExpressionTest, ColumnIndexOutOfBounds)
{
    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(8),
            IntConstant(10)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotExpressionTest, VeryLargeColumnIndex)
{
    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(std::numeric_limits<std::size_t>::max()),
            IntConstant(10)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

TEST(NotExpressionTest, InvalidRightColumn)
{
    NotExpression expression(
        std::make_unique<EqualExpression>(
            IntConstant(10),
            Column(100)
        )
    );

    EXPECT_THROW(
        expression.Eval(testRow),
        std::exception
    );
}

// --------------------------------------------------
// Overflow propagation
// --------------------------------------------------

TEST(NotExpressionTest, AdditionOverflowPropagates)
{
    NotExpression expression(
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

TEST(NotExpressionTest, SubtractionUnderflowPropagates)
{
    NotExpression expression(
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
// Nested logical expressions
// --------------------------------------------------

TEST(NotExpressionTest, NotNestedAndOrTrue)
{
    // NOT ((true AND false) OR false)
    // NOT false -> true

    NotExpression expression(
        std::make_unique<OrExpression>(
            std::make_unique<AndExpression>(
                BoolExpression(true),
                BoolExpression(false)
            ),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_TRUE(std::get<bool>(result));
}

TEST(NotExpressionTest, NotNestedAndOrFalse)
{
    // NOT ((true AND true) OR false)
    // NOT true -> false

    NotExpression expression(
        std::make_unique<OrExpression>(
            std::make_unique<AndExpression>(
                BoolExpression(true),
                BoolExpression(true)
            ),
            BoolExpression(false)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    ASSERT_TRUE(std::holds_alternative<bool>(result));
    EXPECT_FALSE(std::get<bool>(result));
}

// --------------------------------------------------
// De Morgan's laws
// --------------------------------------------------

TEST(NotExpressionTest, DeMorganAnd)
{
    // NOT (A AND B) == (NOT A) OR (NOT B)

    for (bool a : {false, true})
    {
        for (bool b : {false, true})
        {
            NotExpression left(
                std::make_unique<AndExpression>(
                    BoolExpression(a),
                    BoolExpression(b)
                )
            );

            OrExpression right(
                std::make_unique<NotExpression>(
                    BoolExpression(a)
                ),
                std::make_unique<NotExpression>(
                    BoolExpression(b)
                )
            );

            ExpressionVariant leftResult = left.Eval(testRow);
            ExpressionVariant rightResult = right.Eval(testRow);

            ASSERT_TRUE(std::holds_alternative<bool>(leftResult));
            ASSERT_TRUE(std::holds_alternative<bool>(rightResult));

            EXPECT_EQ(
                std::get<bool>(leftResult),
                std::get<bool>(rightResult)
            ) << "a=" << a << ", b=" << b;
        }
    }
}

TEST(NotExpressionTest, DeMorganOr)
{
    // NOT (A OR B) == (NOT A) AND (NOT B)

    for (bool a : {false, true})
    {
        for (bool b : {false, true})
        {
            NotExpression left(
                std::make_unique<OrExpression>(
                    BoolExpression(a),
                    BoolExpression(b)
                )
            );

            AndExpression right(
                std::make_unique<NotExpression>(
                    BoolExpression(a)
                ),
                std::make_unique<NotExpression>(
                    BoolExpression(b)
                )
            );

            ExpressionVariant leftResult = left.Eval(testRow);
            ExpressionVariant rightResult = right.Eval(testRow);

            ASSERT_TRUE(std::holds_alternative<bool>(leftResult));
            ASSERT_TRUE(std::holds_alternative<bool>(rightResult));

            EXPECT_EQ(
                std::get<bool>(leftResult),
                std::get<bool>(rightResult)
            ) << "a=" << a << ", b=" << b;
        }
    }
}

// --------------------------------------------------
// Double negation property
// --------------------------------------------------

TEST(NotExpressionTest, DoubleNegationIdentity)
{
    for (bool value : {false, true})
    {
        NotExpression expression(
            std::make_unique<NotExpression>(
                BoolExpression(value)
            )
        );

        ExpressionVariant result = expression.Eval(testRow);

        ASSERT_TRUE(std::holds_alternative<bool>(result));
        EXPECT_EQ(std::get<bool>(result), value);
    }
}

// --------------------------------------------------
// Input row immutability
// --------------------------------------------------

TEST(NotExpressionTest, IntegerPredicateDoesNotModifyRow)
{
    NotExpression expression(
        std::make_unique<GreaterThanExpression>(
            Column(0),
            Column(2)
        )
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(NotExpressionTest, StringPredicateDoesNotModifyRow)
{
    NotExpression expression(
        std::make_unique<EqualExpression>(
            Column(1),
            Column(5)
        )
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

TEST(NotExpressionTest, NestedPredicateDoesNotModifyRow)
{
    NotExpression expression(
        std::make_unique<AndExpression>(
            std::make_unique<EqualExpression>(
                Column(0),
                Column(4)
            ),
            std::make_unique<LessThanExpression>(
                Column(1),
                Column(5)
            )
        )
    );

    const auto originalValues = testTuple.getTupleValues();

    expression.Eval(testRow);

    EXPECT_EQ(testTuple.getTupleValues(), originalValues);
}

// --------------------------------------------------
// Boolean return type
// --------------------------------------------------

TEST(NotExpressionTest, NotTrueReturnsBoolean)
{
    NotExpression expression(
        BoolExpression(true)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(NotExpressionTest, NotFalseReturnsBoolean)
{
    NotExpression expression(
        BoolExpression(false)
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}

TEST(NotExpressionTest, NestedNotReturnsBoolean)
{
    NotExpression expression(
        std::make_unique<NotExpression>(
            BoolExpression(true)
        )
    );

    ExpressionVariant result = expression.Eval(testRow);

    EXPECT_TRUE(std::holds_alternative<bool>(result));
}
