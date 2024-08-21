#include "Core/Application.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <memory>

void omp::Application::start()
{
    using namespace std::chrono;

    preInit();
    init();

    //float ms_limit = static_cast<float>(1.f / m_FrameLimit);

    time_point previous = steady_clock::now();

    while (!m_RequestExit)
    {
        time_point current_time = steady_clock::now();
        float delta = static_cast<float>(duration_cast<milliseconds>(current_time - previous).count());
        previous = current_time;

        float delta_seconds = delta / 1000.f;
        
        tick(delta_seconds);

        if (m_FrameLimit > 0)
        {

        }
    }

    preDestroy();
}

void omp::Application::requestExit()
{
    m_RequestExit = true;
}

omp::Application::Application(const std::string& flags)
{
    parseFlags(flags);
}

void omp::Application::preInit()
{
    if (m_ThreadCount > 0)
    {
        m_ThreadPool = std::make_unique<omp::ThreadPool>(static_cast<unsigned int>(m_ThreadCount));
    }
    else
    {
        m_ThreadPool = std::make_unique<omp::ThreadPool>();
    }

    m_AssetManager = std::make_unique<omp::AssetManager>(m_ThreadPool.get());
    std::future<bool> wait_assets = m_AssetManager->loadProject();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_Window = glfwCreateWindow(m_Width, m_Height, "VulkanApplication", nullptr, nullptr);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, windowResizeCallback);

    m_Renderer = std::make_unique<omp::Renderer>();
    m_Renderer->initVulkan(m_Window, m_Width, m_Height);
    m_Renderer->initResources();

    wait_assets.wait();
}

void omp::Application::init()
{
    //debug_createSceneManually();
    //m_CurrentScene->setCurrentCamera(0);
    std::weak_ptr<omp::Asset> scene_weak_ptr = m_AssetManager->loadAsset("../assets/main_scene.json");
    if (!scene_weak_ptr.expired())
    {
        m_CurrentScene = scene_weak_ptr.lock()->getObjectAs<omp::Scene>();
    }
    else
    {
        WARN(LogAssetManager, "Application cant load default scene!");
    }

    m_Renderer->loadScene(m_CurrentScene.get());
}

void omp::Application::preDestroy()
{
    std::future<bool> wait_save = m_AssetManager->saveProject();
    wait_save.wait();

    m_ThreadPool.reset();

    m_CurrentScene.reset();
    m_AssetManager.reset();

    m_Renderer->cleanup();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void omp::Application::tick(float delta)
{
    glfwPollEvents();
    m_RequestExit = m_RequestExit || glfwWindowShouldClose(m_Window);

    // Renderer only when not minimized
    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    if (width != 0 && height != 0)
    {
        // TODO: render stuff
        UIData ui_data{};
        ui_data.scene = m_CurrentScene.get();
        m_UIController.update(ui_data, delta);
        m_Renderer->requestDrawFrame(delta);
    }

    // We can run some systems in minimazed application

}

void omp::Application::parseFlags(const std::string& /*commands*/)
{

}

void omp::Application::windowResizeCallback(GLFWwindow* window, int width, int height)
{
    omp::Application* app = reinterpret_cast<omp::Application*>(glfwGetWindowUserPointer(window));

    app->m_Renderer->onWindowResize(width, height);
}

void omp::Application::debug_createSceneManually()
{
    omp::AssetHandle scene_handle = m_AssetManager->createAsset("main_scene", "../assets/main_scene.json", "Scene");
    m_CurrentScene = m_AssetManager->getAsset(scene_handle).lock()->getObjectAs<omp::Scene>();

    const std::string ModelPath = "../models/cube2.obj";
    const std::string TexturePath = "../textures/container.png";
    const std::string TextureSpecular = "../textures/container_specular.png";

    { // START BLOCK TO REUSE NAME
    omp::AssetHandle texture_handle = m_AssetManager->createAsset("container", "../assets/texture.json", "TextureSrc");
    auto texture = m_AssetManager->getAsset(texture_handle).lock()->getObjectAs<omp::TextureSrc>();
    if (texture)
    {
        texture->setPath(TexturePath);
    }
    omp::AssetHandle spec_texture_handle = m_AssetManager->createAsset("container_specular", "../assets/texture_specular.json", "TextureSrc");
    auto spec_texture = m_AssetManager->getAsset(spec_texture_handle).lock()->getObjectAs<omp::TextureSrc>();
    if (spec_texture)
    {
        spec_texture->setPath(TextureSpecular);
    }

    omp::AssetHandle model_handle = m_AssetManager->createAsset("cube_model", "../assets/cube_model.json", "Model");
    auto model = m_AssetManager->getAsset(model_handle).lock()->getObjectAs<omp::Model>();
    if (model)
    {
        model->setPath(ModelPath);
    }

    omp::AssetHandle material_handle = m_AssetManager->createAsset("def_mat", "../assets/def_material.json", "Material");
    auto material = m_AssetManager->getAsset(material_handle).lock()->getObjectAs<omp::Material>();
    material->addSpecularTexture(texture);
    material->addTexture(spec_texture);
    material->addDiffusiveTexture(spec_texture);
    material->setShaderName("Light");
    
    glm::vec3 def_pos = {10.f, 3.f, 4.f};

    for (size_t i = 0; i < 6; i++)
    {
        def_pos.z += 10.f;
        float temp_x = 10.f;
        for (size_t j = 0; j < 6; j++)
        {
            std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>(model, material);
            temp_x += 10.f;
            def_pos.x = temp_x;
            std::string name = std::to_string(i) + "-" + std::to_string(j);
            std::unique_ptr<omp::SceneEntity> entity = std::make_unique<omp::SceneEntity>(name, inst);
            entity->setTranslation(def_pos);
            m_CurrentScene->addEntityToScene(std::move(entity)); 
        }
    }

    std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>(model, material);
    std::unique_ptr<omp::Camera> camera = std::make_unique<omp::Camera>();
    camera->setModelInstance(inst);
    camera->setName("camera1");
    m_CurrentScene->addCameraToScene(std::move(camera));

    }// END BLOCK TO REUSE NAMES
    
    omp::AssetHandle vik_texture_handle = m_AssetManager->createAsset("viking_texture", "../assets/viking_texture.json", "TextureSrc");
    auto vik_texture = m_AssetManager->getAsset(vik_texture_handle).lock()->getObjectAs<omp::TextureSrc>();
    if (vik_texture)
    {
        vik_texture->setPath("../textures/viking.png");
    }

    omp::AssetHandle vik_material_handle = m_AssetManager->createAsset("vik_mat", "../assets/vik_material.json", "Material");
    auto viking_material = m_AssetManager->getAsset(vik_material_handle).lock()->getObjectAs<omp::Material>();
    viking_material->addSpecularTexture(vik_texture);
    viking_material->addTexture(vik_texture);
    viking_material->addDiffusiveTexture(vik_texture);
    viking_material->setShaderName("Light");

    omp::AssetHandle vik_model_handle = m_AssetManager->createAsset("viking_model", "../assets/viking_model.json", "Model");
    auto viking_model = m_AssetManager->getAsset(vik_model_handle).lock()->getObjectAs<omp::Model>();
    if (viking_model)
    {
        viking_model->setPath("../models/vikingroom.obj");
    }
    std::shared_ptr<omp::ModelInstance> viking_inst = std::make_shared<omp::ModelInstance>(viking_model, viking_material);

    std::unique_ptr<omp::SceneEntity> vik_entity = std::make_unique<omp::SceneEntity>("viking_house", viking_inst);
    m_CurrentScene->addEntityToScene(std::move(vik_entity));

    // GRASS
    {
        omp::AssetHandle texture_handle = m_AssetManager->createAsset("grass", "../assets/grass_texture.json", "TextureSrc");
        auto texture = m_AssetManager->getAsset(texture_handle).lock()->getObjectAs<omp::TextureSrc>();
        if (texture)
        {
            texture->setPath("../textures/grass.png");
        }

        omp::AssetHandle model_handle = m_AssetManager->createAsset("quad_model", "../assets/quad_model.json", "Model");
        auto model = m_AssetManager->getAsset(model_handle).lock()->getObjectAs<omp::Model>();
        if (model)
        {
            model->setPath("../models/quad.obj");
        }

        omp::AssetHandle material_handle = m_AssetManager->createAsset("grass_mat", "../assets/grass_material.json", "Material");
        auto material = m_AssetManager->getAsset(material_handle).lock()->getObjectAs<omp::Material>();
        material->addSpecularTexture(texture);
        material->addTexture(texture);
        material->addDiffusiveTexture(texture);
        material->setShaderName("Grass");
        material->enableBlending(true);
        
        std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>(model, material);

        std::unique_ptr<omp::SceneEntity> entity = std::make_unique<omp::SceneEntity>("grass", inst);
        m_CurrentScene->addEntityToScene(std::move(entity)); 
    }
    // WINDOW
    {
        omp::AssetHandle texture_handle = m_AssetManager->createAsset("window_texture", "../assets/window_texture.json", "TextureSrc");
        auto texture = m_AssetManager->getAsset(texture_handle).lock()->getObjectAs<omp::TextureSrc>();
        if (texture)
        {
            texture->setPath("../textures/window.png");
        }

        omp::AssetHandle model_handle = m_AssetManager->createAsset("window_model", "../assets/window_model.json", "Model");
        auto model = m_AssetManager->getAsset(model_handle).lock()->getObjectAs<omp::Model>();
        if (model)
        {
            model->setPath("../models/quad.obj");
        }

        omp::AssetHandle material_handle = m_AssetManager->createAsset("window_mat", "../assets/window_material.json", "Material");
        auto material = m_AssetManager->getAsset(material_handle).lock()->getObjectAs<omp::Material>();
        material->addSpecularTexture(texture);
        material->addTexture(texture);
        material->addDiffusiveTexture(texture);
        material->setShaderName("Grass");
        material->enableBlending(true);
        
        std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>(model, material);

        std::unique_ptr<omp::SceneEntity> entity = std::make_unique<omp::SceneEntity>("window", inst);
        m_CurrentScene->addEntityToScene(std::move(entity)); 
        //grass_mat->setShaderName("Grass");
        //grass_mat->enableBlending(true);
    }
    // PLANE
    {
        omp::AssetHandle texture_handle = m_AssetManager->createAsset("plane_texture", "../assets/plane_texture.json", "TextureSrc");
        auto texture = m_AssetManager->getAsset(texture_handle).lock()->getObjectAs<omp::TextureSrc>();
        if (texture)
        {
            texture->setPath("../textures/default.png");
        }

        omp::AssetHandle model_handle = m_AssetManager->createAsset("plane_model", "../assets/plane_model.json", "Model");
        auto model = m_AssetManager->getAsset(model_handle).lock()->getObjectAs<omp::Model>();
        if (model)
        {
            model->setPath("../models/plane.obj");
        }

        omp::AssetHandle material_handle = m_AssetManager->createAsset("plane_mat", "../assets/plane_material.json", "Material");
        auto material = m_AssetManager->getAsset(material_handle).lock()->getObjectAs<omp::Material>();
        material->addSpecularTexture(texture);
        material->addTexture(texture);
        material->addDiffusiveTexture(texture);
        material->setShaderName("Light");
        
        std::shared_ptr<omp::ModelInstance> inst = std::make_shared<omp::ModelInstance>(model, material);

        std::unique_ptr<omp::SceneEntity> entity = std::make_unique<omp::SceneEntity>("Plane", inst);
        m_CurrentScene->addEntityToScene(std::move(entity)); 
        //plane_mat->setShaderName("Light");
    }

    std::future<bool> future = m_AssetManager->saveProject();
    future.get();
}

void omp::Application::debug_addLightToScene()
{
    const std::string_view model_path = "../models/sphere.obj";

    auto material = m_AssetManager->getAsset("../assets/plane_material.json").lock()->getObjectAs<omp::Material>();
    auto model = m_AssetManager->getAsset("../assets/plane_model.json").lock()->getObjectAs<omp::Model>();

    std::unique_ptr<LightBase>&& global_one = std::make_unique<omp::LightObject<omp::GlobalLight>>(
            "global", std::make_shared<omp::ModelInstance>(model, material));
    m_CurrentScene->addLightToScene(std::move(global_one));

    std::unique_ptr<LightBase>&& point_one = std::make_unique<omp::LightObject<omp::PointLight>>(
            "point 1", std::make_shared<omp::ModelInstance>(model, material));
    m_CurrentScene->addLightToScene(std::move(point_one));

    auto&& point_two = std::make_unique<omp::LightObject<omp::PointLight>>(
            "point 2", std::make_shared<omp::ModelInstance>(model, material));
    m_CurrentScene->addLightToScene(std::move(point_two));

    auto&& spot_one = std::make_unique<omp::LightObject<omp::SpotLight>>(
            "spot 1", std::make_shared<omp::ModelInstance>(model, material));
    m_CurrentScene->addLightToScene(std::move(spot_one));

    auto&& spot_two = std::make_unique<omp::LightObject<omp::SpotLight>>(
            "spot 2", std::make_shared<omp::ModelInstance>(model, material));
    m_CurrentScene->addLightToScene(std::move(spot_two));

    auto&& spot_three = std::make_unique<omp::LightObject<omp::SpotLight>>(
            "spot 3", std::make_shared<omp::ModelInstance>(model, material));
    m_CurrentScene->addLightToScene(std::move(spot_three));

    glm::vec3 light_pos = {10.f, 13.f, 4.f};

    for (auto& light : m_CurrentScene->getLights())
    {
        light->getModelInstance()->getPosition() = light_pos;
        light_pos.x += 10.f;
    }
}

