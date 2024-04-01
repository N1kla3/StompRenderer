#include "gtest/gtest.h"
#include "AssetSystem/AssetManager.h"

const std::string g_TestProjectPath = "../../../tests/";

TEST(AssetLoading, AssetLoaderTest)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST(AssetLoading, AssetMetadata)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST(AssetLoading, AssetFirst)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}
