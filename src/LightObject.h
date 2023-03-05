#pragma once

#include "Rendering/Model.h"
#include "Light.h"

namespace omp
{
    class LightObject
    {
    private:
        std::shared_ptr<Light> m_LightRef;
        std::shared_ptr<Model> m_Model;

    public:
        void updateLightObject();
        void setModel(const std::shared_ptr<Model>& inModel);
        void setLight(const std::shared_ptr<Light>& inLight);
        std::shared_ptr<Light> getLight();
    };
}
