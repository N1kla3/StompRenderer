#include "gtest/gtest.h"
#include <future>
#include "Logs.h"
#include "Async/threadsafe_map.h"
#include "Async/ThreadPool.h"

class SafeMapSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

struct test_value
{
    int a;
    float b;
    std::string name;
    std::vector<std::string> array;

    test_value() = default;
    test_value(const std::string& inname)
        : a(1)
        , b(2.f)
        , name(inname)
        , array({}){}
};


TEST_F(SafeMapSuite, SafeMap_one)
{
    std::unique_ptr<omp::ThreadPool> pool = std::make_unique<omp::ThreadPool>(5);
    omp::threadsafe_map<std::string, test_value> registry;
    
    std::promise<void> start, prom_a, prom_b, prom_c;
    std::shared_future<void> ready = start.get_future();
    std::future<bool> res1, res2, res3;

    res1 = pool->submit([&registry, ready, &prom_a]() -> bool{
        prom_a.set_value();
        ready.wait();

        registry.add_or_update_mapping("one", test_value("asf"));
        registry.add_or_update_mapping("two", test_value("HUH"));
        registry.remove_mapping("one");
        registry.add_or_update_mapping("three", test_value("dsfsd"));

        return true;
    });

    res2 = pool->submit([&registry, ready, &prom_b]() -> bool{
        prom_b.set_value();
        ready.wait();

        registry.add_or_update_mapping("sdf", test_value{});
        registry.add_or_update_mapping("one", test_value("asf"));
        registry.add_or_update_mapping("two", test_value("HUH"));
        registry.remove_mapping("one");
        registry.add_or_update_mapping("three", test_value("dsfsd"));
        registry.remove_mapping("two");
        registry.remove_mapping("three");

        return true;
    });

    res3 = pool->submit([&registry, ready, &prom_c]() -> bool{
        prom_c.set_value();
        ready.wait();

        registry.add_or_update_mapping("df", test_value{"test"});
        registry.add_or_update_mapping("one", test_value("asf"));
        registry.add_or_update_mapping("two", test_value("HUH"));
        registry.add_or_update_mapping("three", test_value("SHUT"));
        registry.remove_mapping("one");
        registry.add_or_update_mapping("three", test_value("dsfsd"));
        registry.remove_mapping("two");

        return true;
    });

    auto st = registry.value_for("df");
    // EXPECT_STREQ(st.name.c_str(), "test");
    registry.remove_mapping("sdf");
    registry.add_or_update_mapping("dfsdf", test_value{});

    EXPECT_NO_THROW(prom_a.get_future().get());
    EXPECT_NO_THROW(prom_b.get_future().get());
    EXPECT_NO_THROW(prom_c.get_future().get());
    start.set_value();

    EXPECT_NO_THROW(res1.get());
    EXPECT_NO_THROW(res2.get());
    EXPECT_NO_THROW(res3.get());

    ASSERT_TRUE(true);
}

TEST_F(SafeMapSuite, SafeMap_two)
{
    // TODO: Write tests when compiling is successfull
}

TEST_F(SafeMapSuite, SafeMap_three)
{
    // TODO: Write tests when compiling is successfull
}

TEST_F(SafeMapSuite, SafeMap_four)
{
    // TODO: Write tests when compiling is successfull
}
