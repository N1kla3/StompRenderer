#pragma once
#include "Model.h"
#include "Light.h"

namespace omp{
class LightObject
{
private:
    std::shared_ptr<Light> m_LightRef;
    std::shared_ptr<Model> m_Model;

public:
    void UpdateLightObject();
    void SetModel(const std::shared_ptr<Model>& inModel);
    std::shared_ptr<Light> GetLight();
};
}
