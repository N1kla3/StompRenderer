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

class TestICommand : public omp::ICommand
{
public:
    TestICommand(int& data) : test_data(&data){}
    virtual void execute() override
    {
        prev_data = *test_data;
        *test_data = 8;
    }
    virtual void undo() override
    {
        *test_data = prev_data;
    }

private: 
    int* test_data;
    int prev_data;
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
    int number = 20;
    omp::CommandStack stack;
    EXPECT_NO_THROW(stack.execute<TestICommand>(number));
    EXPECT_EQ(number, 8);
    EXPECT_NO_THROW(stack.undo());
    EXPECT_EQ(number, 20);
    EXPECT_NO_THROW(stack.undo());
    EXPECT_EQ(number, 20);
    EXPECT_NO_THROW(stack.redo());
    EXPECT_EQ(number, 8);
    EXPECT_NO_THROW(stack.redo());
    ASSERT_EQ(number, 8);
}

TEST_F(CoreTestSuite, ProxyCommandTest)
{
    int number = 20;
    auto stack = std::make_shared<omp::CommandStack>();
    omp::CommandStackProxy proxy(stack);
    EXPECT_NO_THROW(proxy.execute<TestICommand>(number));
    ASSERT_EQ(number, 8);
}
