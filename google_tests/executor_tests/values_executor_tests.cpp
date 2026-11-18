
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "values_executor.hpp"
#include "tuple.hpp"
#include "schema.hpp"
#include "basic_types.hpp"

// ---------------------------------------------------------
// Test Fixtures
// ---------------------------------------------------------

class ValuesExecutorTest : public ::testing::Test {
protected:
    std::shared_ptr<const Schema> schema;
    std::shared_ptr<const std::vector<Row>> rows;

    void SetUp() override {
        schema = std::make_shared<const Schema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"name", FieldType::STRING32}
            }
        );

        rows = std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"}
                })},
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"}
                })},
                Row{Tuple({
                    int64_t{3},
                    std::string{"Carol"}
                })}
            }
        );
    }

    static void ExpectRow(
        const Row& row,
        int64_t expectedID,
        const std::string& expectedName
    ) {
        const auto& values = row.tuple.getTupleValues();

        ASSERT_EQ(values.size(), 2);

        ASSERT_TRUE(std::holds_alternative<int64_t>(values[0]));
        ASSERT_TRUE(std::holds_alternative<std::string>(values[1]));

        EXPECT_EQ(std::get<int64_t>(values[0]), expectedID);
        EXPECT_EQ(std::get<std::string>(values[1]), expectedName);
    }
};

// ---------------------------------------------------------
// 1. Returns rows in their original order
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, ReturnsRowsInOrder) {
    ValuesExecutor executor(rows, schema);

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 2, "Bob");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 3, "Carol");

    executor.Close();
}

// ---------------------------------------------------------
// 2. Next returns false after exhaustion
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, ExhaustionReturnsFalse) {
    ValuesExecutor executor(rows, schema);

    executor.Init();

    Row result;

    EXPECT_TRUE(executor.Next(result));
    EXPECT_TRUE(executor.Next(result));
    EXPECT_TRUE(executor.Next(result));

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ---------------------------------------------------------
// 3. Init resets the executor
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, InitResetsIteration) {
    ValuesExecutor executor(rows, schema);

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 2, "Bob");

    executor.Close();

    executor.Init();

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 2, "Bob");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 3, "Carol");

    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ---------------------------------------------------------
// 4. Empty input returns no rows
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, EmptyInput) {
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    ValuesExecutor executor(emptyRows, schema);

    executor.Init();

    Row result;

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ---------------------------------------------------------
// 5. Empty input still exposes its schema
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, EmptyInputHasSchema) {
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    ValuesExecutor executor(emptyRows, schema);

    const Schema& outputSchema = executor.OutputSchema();

    const auto& fields = outputSchema.getFieldList();

    ASSERT_EQ(fields.size(), 2);

    EXPECT_EQ(fields[0].name, "id");
    EXPECT_EQ(fields[0].type, FieldType::INT64);

    EXPECT_EQ(fields[1].name, "name");
    EXPECT_EQ(fields[1].type, FieldType::STRING32);

    executor.Init();

    Row result;
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ---------------------------------------------------------
// 6. Close before exhaustion
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, CloseBeforeExhaustion) {
    ValuesExecutor executor(rows, schema);

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    executor.Close();

    // Closing early must not prevent reinitialization.
    executor.Init();

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    executor.Close();
}

// ---------------------------------------------------------
// 7. Close can be called multiple times
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, CloseIsIdempotent) {
    ValuesExecutor executor(rows, schema);

    executor.Init();

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    executor.Close();
}

// ---------------------------------------------------------
// 8. Returned rows own their values
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, ReturnedRowsRemainValid) {
    ValuesExecutor executor(rows, schema);

    executor.Init();

    Row firstRow;
    Row secondRow;

    ASSERT_TRUE(executor.Next(firstRow));
    ASSERT_TRUE(executor.Next(secondRow));

    // The first row must not change when Next is called again.
    ExpectRow(firstRow, 1, "Alice");
    ExpectRow(secondRow, 2, "Bob");

    executor.Close();

    // Rows must remain valid after Close.
    ExpectRow(firstRow, 1, "Alice");
    ExpectRow(secondRow, 2, "Bob");
}

// ---------------------------------------------------------
// 9. Two executors can share the same input
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, IndependentIteration) {
    ValuesExecutor executor1(rows, schema);
    ValuesExecutor executor2(rows, schema);

    executor1.Init();
    executor2.Init();

    Row result1;
    Row result2;

    ASSERT_TRUE(executor1.Next(result1));
    ExpectRow(result1, 1, "Alice");

    ASSERT_TRUE(executor1.Next(result1));
    ExpectRow(result1, 2, "Bob");

    // Executor 2 must still start at its own first row.
    ASSERT_TRUE(executor2.Next(result2));
    ExpectRow(result2, 1, "Alice");

    ASSERT_TRUE(executor1.Next(result1));
    ExpectRow(result1, 3, "Carol");

    ASSERT_TRUE(executor2.Next(result2));
    ExpectRow(result2, 2, "Bob");

    executor1.Close();
    executor2.Close();
}

// ---------------------------------------------------------
// 10. Executor retains shared ownership of its input
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, RetainsSharedOwnership) {
    ValuesExecutor executor(rows, schema);

    // Release the fixture's references.
    rows.reset();
    schema.reset();

    // The executor must still own both objects.
    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    EXPECT_EQ(executor.OutputSchema().getFieldList().size(), 2);

    executor.Close();
}

// ---------------------------------------------------------
// 11. Close before Init is safe
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, CloseBeforeInit) {
    ValuesExecutor executor(rows, schema);

    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    executor.Close();
}

// ---------------------------------------------------------
// 12. Duplicate rows are preserved
// ---------------------------------------------------------

TEST_F(ValuesExecutorTest, PreservesDuplicates) {
    auto duplicateRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({int64_t{1}, std::string{"Alice"}})},
                Row{Tuple({int64_t{1}, std::string{"Alice"}})},
                Row{Tuple({int64_t{2}, std::string{"Bob"}})}
            }
        );

    ValuesExecutor executor(duplicateRows, schema);

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 1, "Alice");

    ASSERT_TRUE(executor.Next(result));
    ExpectRow(result, 2, "Bob");

    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}
