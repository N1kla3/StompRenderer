#include "Scene.h"
#include "Logs.h"
#include "Core/CoreLib.h"
#include "SceneEntityFactory.h"

std::vector<std::unique_ptr<omp::SceneEntity>>& omp::Scene::getEntities()
{
    return m_Entities;
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
        entity->TryLoadToGpu(context);
    }

    for (auto& camera : m_Cameras)
    {
        camera->TryLoadToGpu(context);
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
    for (std::unique_ptr<omp::SceneEntity>& camera : m_Cameras)
    {
        names.push_back(camera->getName());

        JsonParser<> new_parser;
        new_parser.writeValue("ClassName", camera->getClassName());
        camera->onSceneSave(new_parser, this);

        parser.writeObject(camera->getName(), std::move(new_parser));
    }
    parser.writeValue("CameraNames", names);
}

void omp::Scene::deserialize(JsonParser<>& parser)
{
    std::vector<std::string> names = parser.readValue<std::vector<std::string>>("EntityNames").value();
    size_t entities_num = names.size();
    for (size_t i = 0; i < entities_num; i++)
    {
        JsonParser<> local_entity = parser.readObject(names[i]);
        std::string class_name = local_entity.readValue<std::string>("ClassName").value();
        std::unique_ptr<SceneEntity> entity = std::move(omp::SceneEntityFactory::createSceneEntity(class_name));
        entity->onSceneLoad(local_entity, this);
        m_Entities.push_back(std::move(entity));
    }

    names = parser.readValue<std::vector<std::string>>("CameraNames").value();
    size_t camera_num = names.size();
    for (size_t i = 0; i < camera_num; i++)
    {
        JsonParser<> local_entity = parser.readObject(names[i]);
        std::string class_name = local_entity.readValue<std::string>("ClassName").value();
        std::unique_ptr<SceneEntity> camera = std::move(omp::SceneEntityFactory::createSceneEntity(class_name));
        camera->onSceneLoad(local_entity, this);
        m_Cameras.push_back(std::move(camera));
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

omp::SceneEntity* omp::Scene::getEntity(int32_t inId) const
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
    // TODO: should take first camera from array
}

omp::SceneEntity* omp::Scene::getCurrentEntity() const
{
    return getEntity(m_CurrentEntityId);
}

void omp::Scene::setCurrentCamera(uint16_t id)
{
    if (id < m_Cameras.size())
    {
        m_CurrentCamera = dynamic_cast<omp::Camera*>(m_Cameras.at(id).get());
        //OMP_ASSERT(m_CurrentCamera, "Ivalid camera!");
    }
    else
    {
        ERROR(LogRendering, "Invalid camera access");
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
        return dynamic_cast<omp::Camera*>(m_Cameras[0].get());
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

