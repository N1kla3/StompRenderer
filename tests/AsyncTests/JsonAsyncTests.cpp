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
const std::string g_PathFour = "../../../tests/testAssets/four.json";

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
                    INFO(LogTesting, "one done");

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
                    INFO(LogTesting, "two done");
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
                    INFO(LogTesting, "three done");
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
                        INFO(LogTesting, "one done");
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
                        INFO(LogTesting, "two done");
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
                        INFO(LogTesting, "three done");
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
    omp::JsonParser parser;
    std::string a = "text";
    std::string& b = a;
    EXPECT_NO_THROW(parser.writeValue("one", a));
    EXPECT_NO_THROW(parser.writeValue("two", b));
    EXPECT_NO_THROW(parser.writeValue("three", "another text"));
    EXPECT_NO_THROW(parser.writeValue("four", true));
    EXPECT_NO_THROW(parser.writeValue("five", 4));
    EXPECT_NO_THROW(parser.writeValue("six", 4.5f));
    EXPECT_NO_THROW(parser.writeValue("seven", 3.4));
    EXPECT_NO_THROW(parser.writeValue("eight", 'd'));
    std::array<float, 4> arr{ 4.f, 5.f, 6.f, 7.f};
    EXPECT_NO_THROW(parser.writeValue("nine", arr));
    omp::JsonParser nested;
    EXPECT_NO_THROW(nested.writeValue("four", true));
    EXPECT_NO_THROW(nested.writeValue("five", 4));
    EXPECT_NO_THROW(nested.writeValue("six", 4.5f));
    EXPECT_NO_THROW(parser.writeObject("obj", std::move(nested)));
    EXPECT_TRUE(parser.writeToFile(g_PathFour));

    {
        omp::JsonParser read_parser;
        EXPECT_TRUE(read_parser.populateFromFile(g_PathFour));
        EXPECT_STREQ(read_parser.readValue<std::string>("one").value_or("no").c_str(), a.c_str());
        EXPECT_STREQ(read_parser.readValue<std::string>("two").value_or("no").c_str(), b.c_str());
        EXPECT_STREQ(read_parser.readValue<std::string>("three").value_or("no").c_str(), "another text");
        EXPECT_EQ(read_parser.readValue<bool>("four").value_or(false), true);
        EXPECT_EQ(read_parser.readValue<int>("five").value_or(2), 4);
        EXPECT_EQ(read_parser.readValue<float>("six").value_or(1.f), 4.5f);
        EXPECT_EQ(read_parser.readValue<double>("seven").value_or(1.0), 3.4);
        EXPECT_EQ(read_parser.readValue<char>("eight").value_or('a'), 'd');
        omp::JsonParser read_nest = read_parser.readObject("obj");
        EXPECT_EQ(read_nest.readValue<bool>("four").value_or(false), true);
        EXPECT_EQ(read_nest.readValue<int>("five").value_or(2), 4);
        EXPECT_EQ(read_nest.readValue<float>("six").value_or(1.f), 4.5f);
    }
    omp::JsonParser read_parser;
    read_parser = std::move(parser);
    EXPECT_STREQ(read_parser.readValue<std::string>("one").value_or("no").c_str(), a.c_str());
    EXPECT_STREQ(read_parser.readValue<std::string>("two").value_or("no").c_str(), b.c_str());
    EXPECT_STREQ(read_parser.readValue<std::string>("three").value_or("no").c_str(), "another text");
    EXPECT_EQ(read_parser.readValue<bool>("four").value_or(false), true);
    EXPECT_EQ(read_parser.readValue<int>("five").value_or(2), 4);
    EXPECT_EQ(read_parser.readValue<float>("six").value_or(1.f), 4.5f);
    EXPECT_EQ(read_parser.readValue<double>("seven").value_or(1.0), 3.4);
    EXPECT_EQ(read_parser.readValue<char>("eight").value_or('a'), 'd');

    omp::JsonParser new_read_parser(std::move(read_parser));
    EXPECT_STREQ(new_read_parser.readValue<std::string>("one").value_or("no").c_str(), a.c_str());
    EXPECT_STREQ(new_read_parser.readValue<std::string>("two").value_or("no").c_str(), b.c_str());
    EXPECT_STREQ(new_read_parser.readValue<std::string>("three").value_or("no").c_str(), "another text");
    EXPECT_EQ(new_read_parser.readValue<bool>("four").value_or(false), true);
    EXPECT_EQ(new_read_parser.readValue<int>("five").value_or(2), 4);
    EXPECT_EQ(new_read_parser.readValue<float>("six").value_or(1.f), 4.5f);
    EXPECT_EQ(new_read_parser.readValue<double>("seven").value_or(1.0), 3.4);
    EXPECT_EQ(new_read_parser.readValue<char>("eight").value_or('a'), 'd');

    ASSERT_TRUE(true);
}
