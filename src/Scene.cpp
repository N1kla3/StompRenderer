#include "Scene.h"

std::vector<std::shared_ptr<omp::ModelInstance>>& omp::Scene::getModels()
{
    return m_Models;
}

void omp::Scene::addModelToScene(const omp::ModelInstance& modelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(std::make_shared<omp::ModelInstance>(modelToAdd));
}

void omp::Scene::addModelToScene(const std::shared_ptr<omp::ModelInstance>& modelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(std::move(modelToAdd));
}

std::shared_ptr<omp::ModelInstance> omp::Scene::getModel(const std::string& inName)
{
    auto model = std::find_if(m_Models.begin(), m_Models.end(), [&inName](const std::shared_ptr<omp::ModelInstance>& inModel)
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
