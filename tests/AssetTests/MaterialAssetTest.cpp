#include "gtest/gtest.h"
#include "AssetSystem/AssetManager.h"
#include "AssetSystem/MaterialAsset.h"
#include "Logs.h"

const std::string g_ShaderTest = "shader";
const std::vector<std::string> g_TestTexturePaths = { "../textures/container.png", "../textures/viking.png" };
const std::string g_NameOne = "test";
const std::string g_NameTwo = "apple";
const std::string g_PathOne = "../tests/testAssets/test.json";
const std::string g_PathTwo = "../tests/testAssets/sec.json";

class Test_MaterialAsset : public omp::MaterialAsset
{
public:
    void setTexturePaths(const std::vector<std::string>& path) {TexturePaths = path;}
    void setShaderName(const std::string& nae) { ShaderName = nae; }
};

class MaterialAssetSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();

        omp::ObjectFactory::s_CreationMap.insert(ADD_CLASS(Test_MaterialAsset));
    }
};

TEST_F(MaterialAssetSuite, MaterialAsset__Test__FirstSave)
{
    omp::AssetManager& manager = omp::AssetManager::getAssetManager();
    manager.createAsset<Test_MaterialAsset>(g_NameOne, g_PathOne);

    auto&& asset = manager.getAsset(g_PathOne);
    ASSERT_TRUE(asset);

    auto&& my_asset = std::dynamic_pointer_cast<Test_MaterialAsset>(asset);
    ASSERT_TRUE(my_asset);
    my_asset->setName(g_NameTwo);
    my_asset->setTexturePaths(g_TestTexturePaths);
    my_asset->setShaderName(g_ShaderTest);

    ASSERT_TRUE(asset->saveToLastValidPath());
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__SecondSave)
{
    omp::AssetManager& manager = omp::AssetManager::getAssetManager();
    manager.createAsset<Test_MaterialAsset>(g_NameTwo, g_PathTwo);

    auto&& asset = manager.getAsset(g_PathTwo);
    ASSERT_TRUE(asset);

    auto&& my_asset = std::dynamic_pointer_cast<Test_MaterialAsset>(asset);
    ASSERT_TRUE(my_asset);
    my_asset->setName(g_NameOne);

    ASSERT_TRUE(asset->saveToLastValidPath());
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__FirstLoad)
{
    auto& manager = omp::AssetManager::getAssetManager();
    auto&& asset = manager.loadAsset(g_PathOne);

    auto&& asset_casted = std::dynamic_pointer_cast<omp::MaterialAsset>(asset);
    ASSERT_TRUE(asset_casted);
    ASSERT_TRUE(asset_casted->getName() == g_NameTwo);
    ASSERT_TRUE(asset_casted->getTexturePaths() == g_TestTexturePaths);
    ASSERT_TRUE(asset_casted->getShaderName() == g_ShaderTest);
    ASSERT_TRUE(std::dynamic_pointer_cast<Test_MaterialAsset>(asset));
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__SecondLoad)
{
    auto& manager = omp::AssetManager::getAssetManager();
    auto&& asset = manager.loadAsset(g_PathTwo);

    auto&& asset_casted = std::dynamic_pointer_cast<omp::MaterialAsset>(asset);
    ASSERT_TRUE(asset_casted);
    ASSERT_TRUE(asset_casted->getName() == g_NameOne);
    ASSERT_TRUE(asset_casted->getTexturePaths() == std::vector<std::string>{});
    ASSERT_TRUE(asset_casted->getShaderName() == "");
    ASSERT_TRUE(std::dynamic_pointer_cast<Test_MaterialAsset>(asset));
}
