#include "gtest/gtest.h"
#include <future>
#include "Logs.h"
#include "Async/ThreadPool.h"
#include "IO/JsonParser.h"

class JsonAsyncSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

const std::string g_PathOne = "../../../tests/testAssets/one.json";
const std::string g_PathTwo = "../../../tests/testAssets/two.json";
const std::string g_PathThree = "../../../tests/testAssets/three.json";

TEST_F(JsonAsyncSuite, JsonAsync_one)
{
    {
    omp::ThreadPool pool{};
    std::promise<void> start, one, two, three;
    std::shared_future<void> ready = start.get_future();
    std::future<int> res1, res2, res3;

    res1 = pool.submit([ready, &one]() -> int
                {
                    one.set_value();
                    ready.wait();

                    omp::JsonParser parser;
                    parser.writeValue("name", "firstname");
                    parser.writeValue("age", 5);
                    EXPECT_TRUE(parser.writeToFile(g_PathOne));
                    INFO(Testing, "one done");

                    return 1;
                });

    res2 = pool.submit([ready, &two]() -> int
                {
                    two.set_value();
                    ready.wait();

                    omp::JsonParser parser;
                    parser.writeValue("name", "firstname");
                    parser.writeValue("age", 5);
                    EXPECT_TRUE(parser.writeToFile(g_PathTwo));
                    INFO(Testing, "two done");
                    return 1;
                });

    res3 = pool.submit([ready, &three]() -> int
                {
                    three.set_value();
                    ready.wait();

                    omp::JsonParser parser;
                    parser.writeValue("name", "firstname");
                    parser.writeValue("age", 5);
                    EXPECT_TRUE(parser.writeToFile(g_PathThree));
                    INFO(Testing, "three done");
                    return 1;
                });

    one.get_future().wait();
    two.get_future().wait();
    three.get_future().wait();
    start.set_value();

    // wait testing
    EXPECT_NO_THROW(res1.get());
    EXPECT_NO_THROW(res2.get());
    EXPECT_NO_THROW(res3.get());
    }
    ASSERT_TRUE(true);
}

TEST_F(JsonAsyncSuite, JsonAsync_two)
{
    {
        omp::ThreadPool pool{};
        std::promise<void> start, one, two, three;
        std::shared_future<void> ready = start.get_future();
        std::future<int> res1, res2, res3;

        res1 = pool.submit([ready, &one]()-> int
                    {
                        one.set_value();
                        ready.wait();

                        omp::JsonParser parser;
                        EXPECT_TRUE(parser.populateFromFile(g_PathOne));
                        EXPECT_STREQ(parser.readValue<std::string>("name").value_or("dd").c_str(), "firstname");
                        EXPECT_EQ(parser.readValue<int>("age").value_or(1), 5);
                        INFO(Testing, "one done");
                        return 1;
                    });

        res2 = pool.submit([ready, &two]()-> int
                    {
                        two.set_value();
                        ready.wait();

                        omp::JsonParser parser;
                        EXPECT_TRUE(parser.populateFromFile(g_PathTwo));
                        EXPECT_STREQ(parser.readValue<std::string>("name").value_or("dd").c_str(), "firstname");
                        EXPECT_EQ(parser.readValue<int>("age").value_or(1), 5);
                        INFO(Testing, "two done");
                        return 1;
                    });

        res3 = pool.submit([ready, &three]() -> int
                    {
                        three.set_value();
                        ready.wait();

                        omp::JsonParser parser;
                        EXPECT_TRUE(parser.populateFromFile(g_PathThree));
                        EXPECT_STREQ(parser.readValue<std::string>("name").value_or("dd").c_str(), "firstname");
                        EXPECT_EQ(parser.readValue<int>("age").value_or(1), 5);
                        INFO(Testing, "three done");
                        return 1;
                    });

        one.get_future().wait();
        two.get_future().wait();
        three.get_future().wait();
        // Start testing
        start.set_value();

        // wait testing
        EXPECT_NO_THROW(res1.get());
        EXPECT_NO_THROW(res2.get());
        EXPECT_NO_THROW(res3.get());
    }
    ASSERT_TRUE(true);
}

TEST_F(JsonAsyncSuite, JsonAsync_three)
{

    ASSERT_TRUE(true);
}
