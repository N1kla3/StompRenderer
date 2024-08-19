#include "gtest/gtest.h"
#include <memory>
#include "AssetSystem/AssetManager.h"
#include "IO/JsonParser.h"
#include "Rendering/Model.h"
#include "AssetSystem/ObjectFactory.h"

class ModelAssetSuite : public ::testing::Test
{
public:
    std::unique_ptr<omp::AssetManager> manager;

protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
    }
    static void TearDownTestSuite()
    {
    }

    virtual void SetUp() override
    {
        INFO(LogTesting, "Setup");
        manager = std::make_unique<omp::AssetManager>(nullptr);
    }

    virtual void TearDown() override
    {
        manager.reset();
        INFO(LogTesting, "Teardown");
    }
};

TEST_F(ModelAssetSuite, ModelAsset__Test__Serialization)
{
    omp::AssetHandle handle = manager->createAsset("testmodel", "invalid", "Model");
    auto asset = manager->getAsset(handle);
    EXPECT_TRUE(asset.lock()->isLoaded());

    std::shared_ptr<omp::Model> model = asset.lock()->getObjectAs<omp::Model>();

    model->setPath("somepath");
    omp::JsonParser<> parser{};
    EXPECT_NO_THROW(model->serialize(parser));
    std::string raw_data = parser.to_string();
    EXPECT_FALSE(model->isLoaded());

    auto second = std::dynamic_pointer_cast<omp::Model>(omp::ObjectFactory::createSerializableObject("Model"));
    EXPECT_NO_THROW(second->deserialize(parser));
    EXPECT_STREQ(model->getPath().c_str(), second->getPath().c_str());

    omp::JsonParser<> new_parser{};
    EXPECT_NO_THROW(second->serialize(new_parser));
    std::string laststr = new_parser.to_string();

    ASSERT_STREQ(raw_data.c_str(), laststr.c_str());
}

