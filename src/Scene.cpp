#include "Scene.h"

std::vector<std::shared_ptr<omp::SceneEntity>>& omp::Scene::getEntities()
{
    return m_Entities;
}

void omp::Scene::addEntityToScene(const omp::SceneEntity& modelToAdd)
{
    m_StateDirty = true;
    m_Entities.push_back(std::make_shared<omp::SceneEntity>(modelToAdd));
}

void omp::Scene::addEntityToScene(const std::shared_ptr<omp::SceneEntity>& modelToAdd)
{
    m_StateDirty = true;
    m_Entities.push_back(std::move(modelToAdd));
}

void omp::Scene::serialize(JsonParser<>& parser)
{

}

void omp::Scene::deserialize(JsonParser<>& parser)
{
    
}

std::shared_ptr<omp::SceneEntity> omp::Scene::getEntity(const std::string& inName) const
{
    auto res_iter = std::find_if(m_Entities.begin(), m_Entities.end(), [&inName](const std::shared_ptr<omp::SceneEntity>& entity)
    {
        if (inName.compare(entity->getName()) == 0)
        {
            return true;
        }
        return false;
    });
    if (res_iter != m_Entities.end())
    {
        return *res_iter;
    }
    else return nullptr;
}

std::shared_ptr<omp::SceneEntity> omp::Scene::getEntity(int32_t inId) const
{
    auto res_iter = std::find_if(m_Entities.begin(), m_Entities.end(), [&inId](const std::shared_ptr<omp::SceneEntity>& entity)
    {
        if (inId == entity->getId())
        {
            return true;
        }
        return false;
    });
    if (res_iter != m_Entities.end())
    {
        return *res_iter;
    }
    else return nullptr;
}


omp::Scene::Scene()
    : m_CurrentCamera(std::make_shared<omp::Camera>())
{

}

std::shared_ptr <omp::SceneEntity> omp::Scene::getCurrentEntity() const
{
    return getEntity(m_CurrentEntityId);
}
