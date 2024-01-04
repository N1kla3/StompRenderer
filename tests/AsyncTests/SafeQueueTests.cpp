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

}
