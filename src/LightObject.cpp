#include "LightObject.h"

void omp::LightObject::UpdateLightObject()
{
    auto& v = m_Model->GetPosition();
    m_LightRef->m_Position.x = v.x;
    m_LightRef->m_Position.y = v.y;
    m_LightRef->m_Position.z = v.z;
}

void omp::LightObject::SetModel(const std::shared_ptr<Model> &inModel)
{
    m_Model = std::move(inModel);
}

void omp::LightObject::SetLight(const std::shared_ptr<Light> &inLight)
{
    m_LightRef = std::move(inLight);
}

std::shared_ptr<omp::Light> omp::LightObject::GetLight()
{
    return std::shared_ptr<Light>();
}
