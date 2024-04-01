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
    for (std::unique_ptr<omp::SceneEntity>& entity : m_Entities)
    {
        JsonParser<> new_parser;
        entity->getModel();
        entity->getModel()->getMaterialInstance()->getStaticMaterial();
        entity->getModel()->getTransform();
        parser.writeObject(entity->getName(), std::move(new_parser));
    }
    for (std::unique_ptr<omp::Camera>& camera : m_Cameras)
    {
        //camera->getNam
    }
    
}

void omp::Scene::deserialize(JsonParser<>& parser)
{
    
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

