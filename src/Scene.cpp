#include "Scene.h"

std::vector<std::shared_ptr<omp::Model>>& omp::Scene::getModels()
{
    return m_Models;
}

void omp::Scene::addModelToScene(const omp::Model& modelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(std::make_shared<omp::Model>(modelToAdd));
}

void omp::Scene::addModelToScene(const std::shared_ptr<omp::Model>& modelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(std::move(modelToAdd));
}

std::shared_ptr<omp::Model> omp::Scene::getModel(const std::string& inName)
{
    auto model = std::find_if(m_Models.begin(), m_Models.end(), [&inName](const std::shared_ptr<omp::Model>& inModel)
    {
        if (inName.compare(inModel->getName()))
        {
            return true;
        }
        return false;
    });
    return *model;
}
