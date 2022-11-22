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

TEST_F(MaterialAssetSuite, MaterialAsset__Test__Saving)
{
    omp::AssetManager::getAssetManager().createAsset<omp::MaterialAsset>("test", "../tests/testAssets/test.json");
}

TEST_F(MaterialAssetSuite, MaterialAsset__Test__yeah)
{
    omp::AssetManager::getAssetManager().createAsset<omp::MaterialAsset>("test", "../tests/testAssets/sec.json");
}
