#include "gtest/gtest.h"
#include "AssetSystem/Asset.h"
#include "SceneEntity.h"
#include "Scene.h"
#include "SceneEntityFactory.h"
#include "Camera.h"
#include "LightObject.h"
#include "AssetSystem/AssetManager.h"

class AssetObjectSuite : public ::testing::Test
{
public:
    inline static std::unique_ptr<omp::ThreadPool> pool;
    std::unique_ptr<omp::ObjectFactory> factory;
    std::unique_ptr<omp::AssetManager> manager;

protected:
    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
        omp::SceneEntityFactory::registerClass<omp::SceneEntity>("SceneEntity");
        omp::SceneEntityFactory::registerClass<omp::Camera>("Camera");
        omp::SceneEntityFactory::registerClass<omp::LightObject<omp::GlobalLight>>("GlobalLight");
        omp::SceneEntityFactory::registerClass<omp::LightObject<omp::PointLight>>("PointLight");
        omp::SceneEntityFactory::registerClass<omp::LightObject<omp::SpotLight>>("SpotLight");

        pool = std::make_unique<omp::ThreadPool>(4);
    }
    static void TearDownTestSuite()
    {
        pool.reset(nullptr);
    }

    virtual void SetUp() override
    {
        INFO(LogTesting, "Setup");
        factory = std::make_unique<omp::ObjectFactory>();
        manager = std::make_unique<omp::AssetManager>(pool.get(), factory.get());
    }

    virtual void TearDown() override
    {
        manager.reset();
        factory.reset();
        INFO(LogTesting, "Teardown");
    }
};

TEST_F(AssetObjectSuite, TestAssetDefaultCreation_Model)
{
    omp::AssetHandle handle = manager->createAsset("testAsset", "../someAsset.json", "Model");
    EXPECT_FALSE(handle == omp::AssetHandle::INVALID_HANDLE);

    auto asset = manager->getAsset(handle);

    EXPECT_FALSE(asset.expired());

    EXPECT_TRUE(asset.lock()->isLoaded());

    EXPECT_TRUE(asset.lock()->getObject());

    EXPECT_FALSE(asset.lock()->getObjectAs<omp::Asset>());

    ASSERT_TRUE(asset.lock()->getObjectAs<omp::Model>());
}

TEST_F(AssetObjectSuite, TestAssetSamePathCreation_Model)
{
    omp::AssetHandle initial_handle = manager->createAsset("testAsset", "../someAsset.json", "Model");
    EXPECT_FALSE(initial_handle == omp::AssetHandle::INVALID_HANDLE);
    omp::AssetHandle wrongclass_handle = manager->createAsset("wrongtest", "../someAsset.json", "Somehting");
    EXPECT_TRUE(wrongclass_handle == omp::AssetHandle::INVALID_HANDLE);
    omp::AssetHandle wrong_handle = manager->createAsset("wrongtest", "../someAsset.json", "Model");
    ASSERT_TRUE(wrong_handle == omp::AssetHandle::INVALID_HANDLE);
}

