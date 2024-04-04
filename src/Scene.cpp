#include "Scene.h"

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

void omp::Scene::serialize(JsonParser<>& parser)
{
    std::vector<std::string> names;
    names.reserve(m_Entities.size());
    for (std::unique_ptr<omp::SceneEntity>& entity : m_Entities)
    {
        names.push_back(entity->getName());

        JsonParser<> new_parser;
        entity->onSceneSave(new_parser);

        parser.writeObject(entity->getName(), std::move(new_parser));
    }
    parser.writeValue("EntityNames", names);

    names.clear();
    for (std::unique_ptr<omp::Camera>& camera : m_Cameras)
    {
        names.push_back(camera->getName());
        //camera->getNam
    }
    parser.writeValue("CameraNames", names);
    
}

void omp::Scene::deserialize(JsonParser<>& parser)
{
    std::vector<std::string> names = parser.readValue<std::vector<std::string>>("EntityNames").value();
    size_t entities_num = names.size();
    
    for (size_t i = 0; i < entities_num; i++)
    {
        // entities load
        // TODO: how to detect subclasses?
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
