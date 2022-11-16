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
