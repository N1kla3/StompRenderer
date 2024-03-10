#include "gtest/gtest.h"
#include <future>
#include "Logs.h"
#include "Async/threadsafe_map.h"

class SafeMapSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

TEST_F(SafeMapSuite, SafeMap_one)
{

    // TODO: Write tests when compiling is successfull
}
