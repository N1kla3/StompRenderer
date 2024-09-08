#include "Scene.h"
#include "Logs.h"
#include "SceneEntityFactory.h"

std::span<std::unique_ptr<omp::SceneEntity>> omp::Scene::getEntities()
{
    return std::span(m_Entities.begin(), m_Entities.end());
}

std::vector<omp::SceneEntity*> omp::Scene::getEntitiesCopy()
{
    std::vector<omp::SceneEntity*> res;
    res.resize(m_Entities.size());
    for (size_t i = 0; i < m_Entities.size(); i++)
    {
        res[i] = m_Entities[i].get();
    }
    return res;
}

std::span<std::unique_ptr<omp::LightBase>> omp::Scene::getLights()
{
    return std::span(m_Lights.begin(), m_Lights.end());
}

void omp::Scene::addEntityToScene(const omp::SceneEntity& modelToAdd)
{
    m_StateDirty = true;
    m_Entities.push_back(std::make_unique<omp::SceneEntity>(modelToAdd));
}

void omp::Scene::addEntityToScene(std::unique_ptr<omp::SceneEntity>&& modelToAdd)
{
    m_StateDirty = true;
    m_Entities.push_back(std::move(modelToAdd));
}

void omp::Scene::loadToGPU(const std::shared_ptr<omp::VulkanContext>& context)
{
    m_VulkanContext = context;

    for (auto& entity : m_Entities)
    {
        entity->tryLoadToGpu(context);
    }

    for (auto& camera : m_Cameras)
    {
        camera->tryLoadToGpu(context);
    }
}

void omp::Scene::serialize(JsonParser<>& parser)
{
    std::vector<std::string> names;
    names.reserve(m_Entities.size());
    for (std::unique_ptr<omp::SceneEntity>& entity : m_Entities)
    {
        names.push_back(entity->getName());

        JsonParser<> new_parser;
        new_parser.writeValue("ClassName", entity->getClassName());
        entity->onSceneSave(new_parser, this);

        parser.writeObject(entity->getName(), std::move(new_parser));
    }
    parser.writeValue("EntityNames", names);

    names.clear();
    for (std::unique_ptr<omp::Camera>& camera : m_Cameras)
    {
        names.push_back(camera->getName());

        JsonParser<> new_parser;
        new_parser.writeValue("ClassName", camera->getClassName());
        camera->onSceneSave(new_parser, this);

        parser.writeObject(camera->getName(), std::move(new_parser));
    }
    parser.writeValue("CameraNames", names);

    names.clear();
    for (std::unique_ptr<omp::LightBase>& light : m_Lights)
    {
        names.push_back(light->getName());

        JsonParser<> new_parser;
        new_parser.writeValue("ClassName", light->getClassName());
        light->onSceneSave(new_parser, this);

        parser.writeObject(light->getName(), std::move(new_parser));
    }
    parser.writeValue("LightNames", names);
}

void omp::Scene::deserialize(JsonParser<>& parser)
{
    auto entities_opt = parser.readValue<std::vector<std::string>>("EntityNames");
    std::vector<std::string> names{};

    if (entities_opt.has_value())
    {
         names = entities_opt.value();
    }
    size_t entities_num = names.size();
    for (size_t i = 0; i < entities_num; i++)
    {
        JsonParser<> local_entity = parser.readObject(names[i]);
        std::string class_name = std::move(local_entity.readValue<std::string>("ClassName").value());
        std::unique_ptr<SceneEntity> entity = omp::SceneEntityFactory::createSceneEntity(class_name);
        entity->onSceneLoad(local_entity, this);
        m_Entities.push_back(std::move(entity));
    }

    auto cameras_opt = parser.readValue<std::vector<std::string>>("CameraNames");
    names.clear();
    if (cameras_opt.has_value())
    {
        names = cameras_opt.value();
    }
    size_t camera_num = names.size();
    for (size_t i = 0; i < camera_num; i++)
    {
        JsonParser<> local_entity = parser.readObject(names[i]);
        std::string class_name = std::move(local_entity.readValue<std::string>("ClassName").value());
        std::unique_ptr<Camera> camera = omp::SceneEntityFactory::createSceneEntity<omp::Camera>(class_name);
        camera->onSceneLoad(local_entity, this);
        m_Cameras.push_back(std::move(camera));
    }
    
    auto light_ops = parser.readValue<std::vector<std::string>>("LightNames");

    if (light_ops.has_value())
    {
        names = light_ops.value();
    }
    size_t light_num = names.size();
    for (size_t i = 0; i < light_num; i++)
    {
        JsonParser<> local_entity = parser.readObject(names[i]);
        std::string class_name = std::move(local_entity.readValue<std::string>("ClassName").value());
        std::unique_ptr<LightBase> light = omp::SceneEntityFactory::createSceneEntity<omp::LightBase>(class_name);
        light->onSceneLoad(local_entity, this);
        m_Lights.push_back(std::move(light));
    }
}

omp::SceneEntity* omp::Scene::getEntity(const std::string& inName) const
{
    omp::SceneEntity* result = nullptr;
    for (const std::unique_ptr<omp::SceneEntity>& ptr : m_Entities)
    {
        if (inName.compare(ptr->getName()) == 0)
        {
            result = ptr.get();
            return result;
        }
    }
    return result;
}

omp::SceneEntity* omp::Scene::getEntity(uint32_t inId) const
{
    omp::SceneEntity* result = nullptr;
    for (const std::unique_ptr<omp::SceneEntity>& ptr : m_Entities)
    {
        if (inId == ptr->getId())
        {
            result = ptr.get();
            return result;
        }
    }
    return result;
}


omp::Scene::Scene()
    : m_CurrentCamera(nullptr)
{
}

omp::SceneEntity* omp::Scene::getCurrentEntity() const
{
    return getEntity(m_CurrentEntityId);
}

bool omp::Scene::setCurrentCamera(uint16_t index)
{
    if (index < m_Cameras.size())
    {
        m_CurrentCamera = m_Cameras.at(index).get();
        return true;
    }
    else
    {
        ERROR(LogRendering, "Invalid camera access");
        return false;
    }
}

omp::Camera* omp::Scene::getCurrentCamera() const
{
    if (m_CurrentCamera)
    {
        return m_CurrentCamera;
    }
    if (!m_Cameras.empty())
    {
        return m_Cameras[0].get();
    }
    return nullptr;
}

void omp::Scene::addCameraToScene()
{
    m_Cameras.push_back(std::make_unique<omp::Camera>());
}

void omp::Scene::addCameraToScene(std::unique_ptr<omp::Camera>&& camera)
{
    m_Cameras.push_back(std::move(camera));
}

void omp::Scene::addLightToScene(std::unique_ptr<omp::LightBase>&& light)
{
    m_Lights.push_back(std::move(light));
}
