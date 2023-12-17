#include "gtest/gtest.h"
#include <iostream>
#include <future>
#include "Async/ThreadPool.h"

TEST(AsyncSuite, ThreadPool_one)
{
    std::promise<void> start, first_ready, second_ready;
    std::shared_future<void> ready(start.get_future());
    std::future<int> first_done;
    std::future<float> second_done;

    ThreadPool pool{};

    std::string a = "dfdf";
    try
    {
        first_done = pool.submit([&a, ready, &first_ready]() -> int {
                    first_ready.set_value();
                    ready.wait();
                    a.clear();
                    a = "aaa";
                    return 2;
                });
        second_done = pool.submit([&a, ready, &second_ready]() -> float {
                    second_ready.set_value();
                    ready.wait();
                    a.clear();
                    a = "bbb";
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a.append("cc");
                    a[33];
                    return 0.f;
                });

        first_ready.get_future().wait();
        second_ready.get_future().wait();
        start.set_value();
        assert(first_done.get() == 2);
        assert(second_done.get() == 0.f);
        std::cout << a << std::endl;


        std::packaged_task<void()> task([]()
                                        {
                                            while (true)
                                            {
                                                interruption_point();
                                            }
                                        });
        interruptible_thread inter(std::move(task));

        inter.interrupt();
        inter.join();
    }
    catch ( ... )
    {
        std::cout << "this is EXCEPTION" << std::endl;
    }
}
