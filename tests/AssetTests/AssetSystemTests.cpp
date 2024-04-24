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
protected:

    static void SetUpTestSuite()
    {
        omp::InitializeTestLogs();
        omp::SceneEntityFactory::registerClass<omp::SceneEntity>("SceneEntity");
        omp::SceneEntityFactory::registerClass<omp::Camera>("Camera");
        omp::SceneEntityFactory::registerClass<omp::LightObject<omp::GlobalLight>>("GlobalLight");
        omp::SceneEntityFactory::registerClass<omp::LightObject<omp::PointLight>>("PointLight");
        omp::SceneEntityFactory::registerClass<omp::LightObject<omp::SpotLight>>("SpotLight");
    }
};

const std::string g_TestProjectPath = "../../../tests/assets";

TEST_F(AssetSuite, AssetLoaderTest)
{
    INFO(LogTesting, "Start first asset test");
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());
    INFO(LogTesting, "Init manager");

    omp::AssetHandle texture_handle = manager.createAsset("cube", g_TestProjectPath + "/texture.json", "TextureSrc");
    auto texture = manager.getAsset(texture_handle).lock()->getObjectAs<omp::TextureSrc>();
    EXPECT_TRUE(texture);
    if (texture)
    {
        texture->setPath("../../../textures/container.png");
    }

    // TODO: project root automatically added
    omp::AssetHandle model_handle = manager.createAsset("cube_model", g_TestProjectPath + "/cube_model.json", "Model");
    auto model = manager.getAsset(model_handle).lock()->getObjectAs<omp::Model>();
    EXPECT_TRUE(model);
    if (model)
    {
        model->setPath("../../../models/cube2.obj");
    }

    omp::AssetHandle material_handle = manager.createAsset("def_mat", g_TestProjectPath + "/def_material.json", "Material");
    auto material = manager.getAsset(material_handle).lock()->getObjectAs<omp::Material>();
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

    omp::AssetHandle scene_handle = manager.createAsset("main_scene", g_TestProjectPath + "/main_scene.json", "Scene");
    auto scene = manager.getAsset(scene_handle).lock()->getObjectAs<omp::Scene>();
    EXPECT_TRUE(scene);
    if (scene)
    {
        scene->addEntityToScene(std::move(simple));
        scene->addCameraToScene(std::move(camera));
        scene->addEntityToScene(std::move(light));
        scene->addEntityToScene(std::move(lighttwo));
    }

    std::future<bool> wait = manager.saveProject();
    EXPECT_NO_THROW(wait.get());
    INFO(LogTesting, "Finished");
    ASSERT_TRUE(true);
}

TEST_F(AssetSuite, AssetMetadata)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    INFO(LogTesting, "Start loading test");
    std::unique_ptr<omp::AssetManager> manager = std::make_unique<omp::AssetManager>(pool.get(), factory.get());

    std::future<bool> wait = manager->loadProject(g_TestProjectPath);
    EXPECT_NO_THROW(wait.get());

    std::future<bool> result = manager->loadAllAssets();
    EXPECT_NO_THROW(result.get());

    ASSERT_TRUE(true);
}

TEST_F(AssetSuite, AssetFirst)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    std::unique_ptr<omp::AssetManager> manager = std::make_unique<omp::AssetManager>(pool.get(), factory.get());

    std::future<bool> wait = manager->loadProject(g_TestProjectPath);
    EXPECT_NO_THROW(wait.get());

    std::future<bool> res = manager->saveProject();
    EXPECT_NO_THROW(res.get());
    ASSERT_TRUE(true);
}
