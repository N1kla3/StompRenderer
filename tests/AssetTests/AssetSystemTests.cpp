#include "gtest/gtest.h"
#include <memory>
#include "AssetSystem/AssetManager.h"
#include "Logs.h"
#include "SceneEntity.h"
#include "Scene.h"
#include "SceneEntityFactory.h"
#include "Camera.h"
#include "LightObject.h"
#include "Rendering/TextureSrc.h"

class AssetSuite : public ::testing::Test
{
public:
    inline static std::unique_ptr<omp::ThreadPool> pool;
    std::unique_ptr<omp::AssetManager> manager;

protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();

        pool = std::make_unique<omp::ThreadPool>(4);
    }
    static void TearDownTestSuite()
    {
        pool.reset(nullptr);
    }

    virtual void SetUp() override
    {
        INFO(LogTesting, "Setup");
        manager = std::make_unique<omp::AssetManager>(pool.get());
    }

    virtual void TearDown() override
    {
        manager.reset();
        INFO(LogTesting, "Teardown");
    }

};

const std::string g_TestProjectPath = "../../../tests/assets";

TEST_F(AssetSuite, AssetSystem__test__CreatingAssets)
{
    INFO(LogTesting, "Start first asset test");

    omp::AssetHandle texture_handle = manager->createAsset("cube", g_TestProjectPath + "/texture.json", "TextureSrc");
    auto texture = manager->getAsset(texture_handle).lock()->getObjectAs<omp::TextureSrc>();
    EXPECT_TRUE(texture);
    if (texture)
    {
        texture->setPath("../../../textures/container.png");
    }

    // TODO: project root automatically added
    omp::AssetHandle model_handle = manager->createAsset("cube_model", g_TestProjectPath + "/cube_model.json", "Model");
    auto model = manager->getAsset(model_handle).lock()->getObjectAs<omp::Model>();
    EXPECT_TRUE(model);
    if (model)
    {
        model->setPath("../../../models/cube2.obj");
    }

    omp::AssetHandle material_handle = manager->createAsset("def_mat", g_TestProjectPath + "/def_material.json", "Material");
    auto material = manager->getAsset(material_handle).lock()->getObjectAs<omp::Material>();
    material->addSpecularTexture(texture);
    material->addTexture(texture);
    material->addDiffusiveTexture(texture);


    std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>(model, material);
    std::unique_ptr<omp::SceneEntity> simple = std::make_unique<omp::SceneEntity>("testent", inst);
    std::unique_ptr<omp::Camera> camera = std::make_unique<omp::Camera>();
    camera->setModelInstance(inst);
    camera->setName("camera1");
    std::unique_ptr<omp::SceneEntity> light = std::make_unique<omp::LightObject<omp::GlobalLight>>("globallig", inst);
    std::unique_ptr<omp::SceneEntity> lighttwo = std::make_unique<omp::LightObject<omp::SpotLight>>("lightspot", inst);

    omp::AssetHandle scene_handle = manager->createAsset("main_scene", g_TestProjectPath + "/main_scene.json", "Scene");
    auto scene = manager->getAsset(scene_handle).lock()->getObjectAs<omp::Scene>();
    EXPECT_TRUE(scene);
    if (scene)
    {
        scene->addEntityToScene(std::move(simple));
        scene->addCameraToScene(std::move(camera));
        scene->addEntityToScene(std::move(light));
        scene->addEntityToScene(std::move(lighttwo));
    }

    std::future<bool> wait = manager->saveProject();
    EXPECT_NO_THROW(wait.get());
    INFO(LogTesting, "Finished");
    ASSERT_TRUE(true);
}

TEST_F(AssetSuite, AssetManager__test__LoadingAssets)
{
    INFO(LogTesting, "Start loading test");

    std::future<bool> wait = manager->loadProjectAsync(g_TestProjectPath);
    EXPECT_NO_THROW(wait.get());

    std::future<bool> result = manager->loadAllAssets();
    bool res_val;
    EXPECT_NO_THROW(res_val = result.get());

    ASSERT_TRUE(res_val);
}

TEST_F(AssetSuite, AssetManager__test__SavingProject)
{
    std::future<bool> wait = manager->loadProjectAsync(g_TestProjectPath);
    EXPECT_NO_THROW(wait.get());

    std::future<bool> res = manager->saveProject();
    bool res_val;
    EXPECT_NO_THROW(res_val = res.get());
    ASSERT_TRUE(res_val);
}

TEST_F(AssetSuite, AssetManager__test__AssetUnload)
{
    std::future<bool> wait = manager->loadProjectAsync(g_TestProjectPath);
    EXPECT_NO_THROW(wait.get());

    std::shared_ptr<omp::Asset> asset = manager->getAsset(g_TestProjectPath + "/main_scene.json").lock();
    EXPECT_TRUE(asset);
    EXPECT_FALSE(asset->isLoaded());

    auto fut = manager->loadAssetAsync(asset->getMetaData().asset_id);
    EXPECT_NO_THROW(fut.get());
    EXPECT_TRUE(asset->isLoaded());
    EXPECT_TRUE(manager->deleteAsset(asset->getMetaData().asset_id));

    auto asset_deleted = manager->getAsset(g_TestProjectPath + "/main_scene.json");
    EXPECT_TRUE(asset_deleted.expired());

    omp::AssetHandle scene_handle = manager->createAsset("main_scene", g_TestProjectPath + "/main_scene.json", "Scene");
    ASSERT_TRUE(scene_handle.isValid());
}

