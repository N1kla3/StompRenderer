#include "gtest/gtest.h"
#include "AssetSystem/AssetManager.h"
#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/SceneAsset.h"


class Test_SceneAsset : public omp::SceneAsset
{
public:
};

class SceneAssetSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();

        omp::AssetLoader::s_AssetClasses.insert(ADD_CLASS(Test_SceneAsset));
    }
};

const std::string g_Path = "../tests/testAssets/ModelAsset.json";
const std::string g_ModelPath = "../models/cube.obj";
const std::string g_MaterialPath = "../tests/testAssets/test.json";

TEST_F(SceneAssetSuite, ModelAsset__Test__FirstSave)
{
}

TEST_F(SceneAssetSuite, ModelAsset__Test__FirstLoad)
{
}
