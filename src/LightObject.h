#pragma once
#include "Model.h"
#include "Light.h"

namespace omp{
class LightObject : public Model
{
private:
    std::shared_ptr<Light> m_LightRef;

public:

};
}
