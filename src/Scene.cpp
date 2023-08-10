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

std::shared_ptr<omp::SceneEntity> omp::Scene::getEntity(const std::string& inName)
{
    auto model = std::find_if(m_Entities.begin(), m_Entities.end(), [&inName](const std::shared_ptr<omp::SceneEntity>& inModel)
    {
        if (inName.compare(inModel->getName()) == 0)
        {
            return true;
        }
        return false;
    });
    return *model;
}

omp::Scene::Scene()
    : m_CurrentCamera(std::make_shared<omp::Camera>())
{

}
