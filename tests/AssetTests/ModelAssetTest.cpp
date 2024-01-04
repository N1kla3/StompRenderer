#include "gtest/gtest.h"
#include "AssetSystem/AssetManager.h"
#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/ModelAsset.h"

class Test_ModelAsset : public omp::ModelAsset
{
public:
    void setModelPath(const std::string& path) { ModelPath = path; }
    void setMaterialPath(const std::string& path) { MaterialPath = path; }
};

class ModelAssetSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();

        omp::AssetLoader::s_AssetClasses.insert(ADD_CLASS(Test_ModelAsset));
    }
};

const std::string g_Path = "../tests/testAssets/ModelAsset.json";
const std::string g_ModelPath = "../models/cube.obj";
const std::string g_MaterialPath = "../tests/testAssets/test.json";

TEST_F(ModelAssetSuite, ModelAsset__Test__FirstSave)
{
    omp::AssetManager& manager = omp::AssetManager::getAssetManager();

    manager.createAsset<Test_ModelAsset>("TestModel", g_Path);

    auto&& asset = manager.getAsset(g_Path);
    ASSERT_TRUE(asset);

    auto&& my_asset = std::dynamic_pointer_cast<Test_ModelAsset>(asset);
    ASSERT_TRUE(my_asset);
    my_asset->setName("NewTestModel");
    my_asset->setModelPath(g_ModelPath);
    my_asset->setMaterialPath(g_MaterialPath);

    ASSERT_TRUE(asset->saveToLastValidPath());
}

TEST_F(ModelAssetSuite, ModelAsset__Test__FirstLoad)
{
    omp::AssetManager& manager = omp::AssetManager::getAssetManager();
    auto&& asset = manager.loadAsset(g_Path);

    auto&& asset_casted = std::dynamic_pointer_cast<omp::ModelAsset>(asset);
    ASSERT_TRUE(asset_casted);
    ASSERT_TRUE(asset_casted->getName() == "NewTestModel");
    ASSERT_TRUE(asset_casted->getMaterialPath() == g_MaterialPath);
    ASSERT_TRUE(asset_casted->getModelPath() == g_ModelPath);
    ASSERT_TRUE(std::dynamic_pointer_cast<Test_ModelAsset>(asset));
}
