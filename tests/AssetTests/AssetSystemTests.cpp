#include "gtest/gtest.h"
#include <memory>
#include "AssetSystem/AssetManager.h"
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

TEST(AssetSuite, AssetLoaderTest)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    omp::AssetHandle texture_handle = manager.createAsset("cube", "/texture.json", "TextureSrc");
    //manager.getAsset()

    std::unique_ptr<omp::Scene> local_scene = std::make_unique<omp::Scene>();

    std::shared_ptr<omp::TextureSrc> texture = std::make_shared<omp::TextureSrc>("../../../textures/container.png");
    std::shared_ptr<omp::Model> model = std::make_shared<omp::Model>("../../../models/cube2.obj");
    auto material = std::make_shared<omp::Material>();

    std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>();
    std::unique_ptr<omp::SceneEntity> simple = std::make_unique<omp::SceneEntity>("testent", inst);
    std::unique_ptr<omp::Camera> camera = std::make_unique<omp::Camera>();
    std::unique_ptr<omp::SceneEntity> light = std::make_unique<omp::LightObject<omp::GlobalLight>>("globallig", inst);
    std::unique_ptr<omp::SceneEntity> lighttwo = std::make_unique<omp::LightObject<omp::SpotLight>>("lightspot", inst);

    local_scene->addEntityToScene(std::move(simple));
    local_scene->addCameraToScene(std::move(camera));

    manager.createAsset("FirstScene", "", "");

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST(AssetSuite, AssetMetadata)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}

TEST(AssetSuite, AssetFirst)
{
    std::unique_ptr<omp::ThreadPool> pool(std::make_unique<omp::ThreadPool>(4));
    std::unique_ptr<omp::ObjectFactory> factory(std::make_unique<omp::ObjectFactory>());
    omp::AssetManager manager(pool.get(), factory.get());

    std::future<bool> wait = manager.loadProject(g_TestProjectPath);
}
