#include "Scene.h"

std::vector<std::shared_ptr<omp::Model>>& omp::Scene::GetModels()
{
    return m_Models;
}

void omp::Scene::AddModelToScene(const omp::Model &ModelToAdd)
{
    m_StateDirty = true;
    m_Models.push_back(std::make_shared<omp::Model>(ModelToAdd));
}
