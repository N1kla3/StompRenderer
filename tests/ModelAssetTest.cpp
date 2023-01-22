#include "gtest/gtest.h"
#include "AssetManager.h"
#include "ModelAsset.h"

class ModelAssetSuite : public ::testing::Test
{
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
};

class Test_ModelAsset : public omp::ModelAsset
{
public:
    void setModelPath(const std::string& path) { modelPath = path; }
    void setMaterialPath(const std::string& path) { materialPath = path; }
};

TEST_F(ModelAssetSuite, MaterialAsset__Test__FirstSave)
{
    omp::AssetManager& manager = omp::AssetManager::getAssetManager();


}
