#include "gtest/gtest.h"
#include <future>
#include "Logs.h"
#include "Async/threadsafe_queue.h"

class SafeQueueSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

TEST_F(SafeQueueSuite, SafeQueue_one)
{
    std::promise<void> start, first_ready, second_ready, third_ready;
    std::shared_future<void> ready(start.get_future());
    std::future<int> first_done;
    std::future<float> second_done;
    std::future<int> third_done;

    omp::ThreadSafeQueue<float> first_q;
    omp::ThreadSafeQueue<std::string> second_q;

    first_done = std::async(std::launch::async, [&first_q, &second_q, ready, &first_ready]() -> int {
        first_ready.set_value();
        ready.wait();

        float a = 4.f;
        first_q.push(12.f);
        first_q.push(3);
        first_q.push(a);

        second_q.push("aa");
        second_q.push("aa");
        second_q.push("aa");
        second_q.push("aa");
        second_q.push("aa");
        second_q.push("aa");

        std::string res;
        EXPECT_TRUE(second_q.tryPop(res));

        return 2;
    });
    second_done = std::async(std::launch::async, [&first_q, &second_q, ready, &second_ready]() -> float {
        second_ready.set_value();
        ready.wait();

        float&& b = 2;
        const float &ref = 3.f;
        first_q.push(b);
        first_q.push(ref);
        float out;
        EXPECT_TRUE(first_q.tryPop(out));

        second_q.push("aa");
        second_q.push("bb");
        second_q.tryPop();
        second_q.tryPop();
        second_q.tryPop();
        second_q.tryPop();
        second_q.tryPop();
        second_q.tryPop();
        second_q.tryPop();

        return 0.f;
    });
    third_done = std::async(std::launch::async, [&first_q, ready, &third_ready]() -> int
    {
        third_ready.set_value();
        ready.wait();

        first_q.push(4.f);
        first_q.push(4.f);
        first_q.push(4.f);
        first_q.push(4.f);
        first_q.push(4.f);

        return -1;
    });

    first_ready.get_future().wait();
    second_ready.get_future().wait();
    third_ready.get_future().wait();
    start.set_value();
    
    EXPECT_EQ(first_done.get(), 2);
    EXPECT_EQ(second_done.get(), 0.f);
    EXPECT_EQ(third_done.get(), -1);
}

TEST_F(SafeQueueSuite, SafeQueue_two)
{
    std::promise<void> start, first_ready, second_ready, third_ready;
    std::shared_future<void> ready(start.get_future());
    std::future<int> first_done;
    std::future<float> second_done;
    std::future<int> third_done;

    omp::ThreadSafeQueue<float> first_q;
    first_q.push(5.f);
    first_q.push(5.f);
    first_q.push(5.f);
    first_q.push(4.f);

    first_done = std::async(std::launch::async, [&first_q, ready, &first_ready]() -> int {
        first_ready.set_value();
        ready.wait();

        float res;
        EXPECT_TRUE(first_q.tryPop(res));
        EXPECT_EQ(res, 5.f);

        return 2;
    });
    second_done = std::async(std::launch::async, [&first_q, ready, &second_ready]() -> float {
        second_ready.set_value();
        ready.wait();

        float res;
        EXPECT_TRUE(first_q.tryPop(res));
        EXPECT_EQ(res, 5.f);

        return 0.f;
    });
    third_done = std::async(std::launch::async, [&first_q, ready, &third_ready]() -> int
    {
        third_ready.set_value();
        ready.wait();

        float res;
        EXPECT_TRUE(first_q.tryPop(res));
        EXPECT_EQ(res, 5.f);

        return -1;
    });

    first_ready.get_future().wait();
    second_ready.get_future().wait();
    third_ready.get_future().wait();
    start.set_value();

    EXPECT_EQ(first_done.get(), 2);
    EXPECT_EQ(second_done.get(), 0.f);
    EXPECT_EQ(third_done.get(), -1);
}

TEST_F(SafeQueueSuite, SafeQueue_three)
{
    std::promise<void> start, first_ready, second_ready, third_ready;
    std::shared_future<void> ready(start.get_future());
    std::future<int> first_done;
    std::future<float> second_done;
    std::future<int> third_done;

    omp::ThreadSafeQueue<float> first_q;
    omp::ThreadSafeQueue<std::string> second_q;

    first_done = std::async(std::launch::async, [&first_q, &second_q, ready, &first_ready]() -> int {
        first_ready.set_value();
        ready.wait();

        first_q.push(4);
        first_q.tryPop();
        second_q.push("abc");
        second_q.tryPop();

        return 2;
    });
    second_done = std::async(std::launch::async, [&first_q, &second_q, ready, &second_ready]() -> float {
        second_ready.set_value();
        ready.wait();

        first_q.push(4);
        first_q.tryPop();
        second_q.push("abc");
        second_q.tryPop();

        return 0.f;
    });
    third_done = std::async(std::launch::async, [&first_q, &second_q, ready, &third_ready]() -> int
    {
        third_ready.set_value();
        ready.wait();

        first_q.push(4);
        first_q.tryPop();
        second_q.push("abc");
        second_q.tryPop();

        return -1;
    });
    first_ready.get_future().wait();
    second_ready.get_future().wait();
    third_ready.get_future().wait();
    start.set_value();

    EXPECT_EQ(first_done.get(), 2);
    EXPECT_EQ(second_done.get(), 0.f);
    EXPECT_EQ(third_done.get(), -1);

    EXPECT_TRUE(first_q.empty());
    EXPECT_TRUE(second_q.empty());
}
