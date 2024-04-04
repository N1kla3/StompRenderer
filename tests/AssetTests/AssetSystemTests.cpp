#include "gtest/gtest.h"
#include "AssetSystem/AssetManager.h"

class AssetSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

const std::string g_TestProjectPath = "../../../tests/assets";

TEST(AssetSuite, AssetLoaderTest)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST(AssetSuite, AssetMetadata)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST(AssetSuite, AssetFirst)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}
