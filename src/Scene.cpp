#include "Scene.h"

std::vector<omp::Model>& omp::Scene::GetModels()
{
    return m_Models;
}

void omp::Scene::AddModelToScene(const omp::Model &ModelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(ModelToAdd);
}
