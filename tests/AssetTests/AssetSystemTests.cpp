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
        std::cout <<"hello";
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
    if (texture)
    {
        texture->setPath("../../../textures/container.png");
    }

    // TODO: project root automatically added
    omp::AssetHandle model_handle = manager.createAsset("cube_model", g_TestProjectPath + "/cube_model.json", "Model");
    auto model = manager.getAsset(texture_handle).lock()->getObjectAs<omp::Model>();
    if (model)
    {
        model->setPath("../../../models/cube2.obj");
    }

    auto material = std::make_shared<omp::Material>();

    std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>();
    std::unique_ptr<omp::SceneEntity> simple = std::make_unique<omp::SceneEntity>("testent", inst);
    std::unique_ptr<omp::Camera> camera = std::make_unique<omp::Camera>();
    std::unique_ptr<omp::SceneEntity> light = std::make_unique<omp::LightObject<omp::GlobalLight>>("globallig", inst);
    std::unique_ptr<omp::SceneEntity> lighttwo = std::make_unique<omp::LightObject<omp::SpotLight>>("lightspot", inst);

    omp::AssetHandle scene_handle = manager.createAsset("main_scene", g_TestProjectPath + "/main_scene.json", "Scene");
    auto scene = manager.getAsset(scene_handle).lock()->getObjectAs<omp::Scene>();
    if (scene)
    {
        scene->addEntityToScene(std::move(simple));
        scene->addCameraToScene(std::move(camera));
        scene->addEntityToScene(std::move(light));
        scene->addEntityToScene(std::move(lighttwo));
    }

    std::future<bool> wait = manager.saveProject();
    INFO(LogTesting, "Finished");
}

TEST_F(AssetSuite, AssetMetadata)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST_F(AssetSuite, AssetFirst)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}
