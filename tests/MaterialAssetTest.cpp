#include "gtest/gtest.h"
#include "AssetManager.h"
#include "MaterialAsset.h"
#include "Logs.h"

class MaterialAssetSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

class Test_MaterialAsset : public omp::MaterialAsset
{
public:
    void setTexturePaths(const std::vector<std::string>& path) {TexturePaths = path;}
    void setShaderName(const std::string& nae) { ShaderName = nae; }
};

TEST_F(MaterialAssetSuite, MaterialAsset__Test__Saving)
{
    const std::string test_shader_name = "shader";
    const std::vector<std::string> test_texture_paths = { "one", "two" };

    omp::AssetManager& manager = omp::AssetManager::getAssetManager();
    manager.createAsset<Test_MaterialAsset>("test", "../tests/testAssets/test.json");
    auto&& asset = manager.getAsset("test");
    auto&& my_asset = std::dynamic_pointer_cast<Test_MaterialAsset>(asset);
    my_asset->setName("uoui");
    my_asset->setTexturePaths(test_texture_paths);
    my_asset->setShaderName(test_shader_name);

    ASSERT_TRUE(asset->saveToLastValidPath());
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__yeah)
{
    omp::AssetManager::getAssetManager().createAsset<omp::MaterialAsset>("test", "../tests/testAssets/sec.json");
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__Loading)
{
    auto& manager = omp::AssetManager::getAssetManager();
    auto&& asset = manager.loadAsset("../tests/testAssets/sec.json");
    ASSERT_TRUE(asset->getName() == "test");
    ASSERT_TRUE(std::dynamic_pointer_cast<omp::MaterialAsset>(asset));
    ASSERT_FALSE(std::dynamic_pointer_cast<Test_MaterialAsset>(asset));
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__inherit)
{

}
