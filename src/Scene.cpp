#include "Scene.h"

const std::vector<omp::Model>& omp::Scene::GetModels() const
{
    return m_Models;
}

void omp::Scene::AddModelToScene(const omp::Model &ModelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(ModelToAdd);
}
