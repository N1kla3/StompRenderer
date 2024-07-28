#include "gtest/gtest.h"
#include "Logs.h"
#include "Core/ICommand.h"
#include "Core/CoreLib.h"

class CoreTestSuite : public ::testing::Test
{
protected: 
    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};


TEST_F(CoreTestSuite, Id64Test)
{
    std::set<uint64_t> set;
    size_t size = 100;
    for (size_t i = 0; i < size; i++)
    {
        auto id = omp::CoreLib::generateId64();
        //INFO(LogTesting, "New id64: {}", id);
        set.insert(id);
    }
    EXPECT_TRUE(set.size() == size);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::set<uint64_t> set_two;
    size = 100;
    for (size_t i = 0; i < size; i++)
    {
        auto id = omp::CoreLib::generateId64();
        //INFO(LogTesting, "New id64: {}", id);
        set_two.insert(id);
    }
    ASSERT_TRUE(set_two.size() == size);
}

TEST_F(CoreTestSuite, Id32Test)
{

    std::set<uint32_t> set;
    size_t size = 100;
    for (size_t i = 0; i < size; i++)
    {
        auto id = omp::CoreLib::generateId32();
        //INFO(LogTesting, "New id32: {}", id);
        set.insert(id);
    }
    EXPECT_TRUE(set.size() == size);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::set<uint32_t> set_two;
    size = 100;
    for (size_t i = 0; i < size; i++)
    {
        auto id = omp::CoreLib::generateId32();
        //INFO(LogTesting, "New id32: {}", id);
        set_two.insert(id);
    }
    ASSERT_TRUE(set_two.size() == size);
}

TEST_F(CoreTestSuite, CommandTest)
{

}

TEST_F(CoreTestSuite, UndoCommandTest)
{
}
