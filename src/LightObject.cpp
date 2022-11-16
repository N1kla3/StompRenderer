#include "LightObject.h"

void omp::LightObject::updateLightObject()
{
    auto& v = m_Model->getPosition();
    m_LightRef->position.x = v.x;
    m_LightRef->position.y = v.y;
    m_LightRef->position.z = v.z;
}

void omp::LightObject::setModel(const std::shared_ptr<Model>& inModel)
{
    m_Model = std::move(inModel);
}

void omp::LightObject::setLight(const std::shared_ptr<Light>& inLight)
{
    m_LightRef = std::move(inLight);
}

std::shared_ptr<omp::Light> omp::LightObject::getLight()
{
    return std::shared_ptr<Light>();
}
