#include "gtest/gtest.h"
#include "AssetSystem/AssetManager.h"
#include "IO/JsonParser.h"
#include "Scene.h"
#include "AssetSystem/ObjectFactory.h"

class SceneAssetSuite : public ::testing::Test
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

TEST_F(SceneAssetSuite, SceneAsset__Test__EmptySerialization)
{
    // PREPARE
    omp::AssetHandle handle = manager->createAsset("testscene", "invalid", "Scene");
    auto asset = manager->getAsset(handle);
    EXPECT_TRUE(asset.lock()->isLoaded());

    std::shared_ptr<omp::Scene> scene = asset.lock()->getObjectAs<omp::Scene>();
    EXPECT_TRUE(scene);

    // EXECUTE
    omp::JsonParser<> parser{};
    EXPECT_NO_THROW(scene->serialize(parser));
    std::string raw_data = parser.to_string();

    auto second = std::dynamic_pointer_cast<omp::Scene>(omp::ObjectFactory::createSerializableObject("Scene"));
    EXPECT_NO_THROW(second->deserialize(parser));
    EXPECT_EQ(scene->getEntities().size(), second->getEntities().size());
    EXPECT_EQ(scene->getLights().size(), second->getLights().size());

    omp::JsonParser<> new_parser{};
    EXPECT_NO_THROW(second->serialize(new_parser));
    std::string laststr = new_parser.to_string();

    ASSERT_STREQ(raw_data.c_str(), laststr.c_str());
}

TEST_F(SceneAssetSuite, SceneAsset__Test__Serialization)
{
    // PREPARE
    omp::AssetHandle handle = manager->createAsset("testscene", "invalid", "Scene");
    auto asset = manager->getAsset(handle);
    EXPECT_TRUE(asset.lock()->isLoaded());

    std::shared_ptr<omp::Scene> scene = asset.lock()->getObjectAs<omp::Scene>();
    EXPECT_TRUE(scene);

    scene->addCameraToScene();
    scene->addLightToScene(std::make_unique<omp::LightObject<omp::GlobalLight>>("light"));
    scene->addEntityToScene(std::make_unique<omp::SceneEntity>("ent", nullptr));

    // EXECUTE
    omp::JsonParser<> parser{};
    EXPECT_NO_THROW(scene->serialize(parser));
    std::string raw_data = parser.to_string();

    auto second = std::dynamic_pointer_cast<omp::Scene>(omp::ObjectFactory::createSerializableObject("Scene"));
    EXPECT_NO_THROW(second->deserialize(parser));
    EXPECT_EQ(scene->getEntities().size(), second->getEntities().size());
    EXPECT_EQ(scene->getLights().size(), second->getLights().size());

    omp::JsonParser<> new_parser{};
    EXPECT_NO_THROW(second->serialize(new_parser));
    std::string laststr = new_parser.to_string();

    ASSERT_STREQ(raw_data.c_str(), laststr.c_str());
}

TEST_F(SceneAssetSuite, SceneAsset__Test__Cameras)
{
    omp::Scene scene{};
    EXPECT_FALSE(scene.getCurrentCamera());
    scene.addCameraToScene();
    EXPECT_TRUE(scene.getCurrentCamera());
    EXPECT_FALSE(scene.setCurrentCamera(4));
    EXPECT_TRUE(scene.getCurrentCamera());
    EXPECT_TRUE(scene.setCurrentCamera(0));
    ASSERT_TRUE(scene.getCurrentCamera());
}

TEST_F(SceneAssetSuite, SceneAsset__Test__Lights)
{
    omp::Scene scene{};
    scene.addLightToScene(std::make_unique<omp::LightObject<omp::GlobalLight>>("glo"));
    scene.addLightToScene(std::make_unique<omp::LightObject<omp::PointLight>>("poi"));
    scene.addLightToScene(std::make_unique<omp::LightObject<omp::SpotLight>>("spo"));

    EXPECT_EQ(scene.getEntities().size(), 0);

    ASSERT_EQ(scene.getLights().size(), 3);
}

TEST_F(SceneAssetSuite, SceneAsset__Test__Entities)
{
    omp::Scene scene{};

    omp::SceneEntity entity{"ent", nullptr};

    uint32_t test_id = entity.getId();
    scene.setCurrentId(test_id);
    EXPECT_EQ(scene.getCurrentId(), test_id);
    
    scene.addEntityToScene(entity);
    scene.addEntityToScene(omp::SceneEntity{});
    
    EXPECT_EQ(scene.getEntities().size(), 2);
    EXPECT_EQ(*scene.getEntity("ent"), entity);
    EXPECT_EQ(*scene.getEntity(test_id), entity);
    ASSERT_EQ(*scene.getCurrentEntity(), entity);
}

