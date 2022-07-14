#pragma once
#include <memory>
#include "ImguiUnit.h"

namespace omp{

class Light;

class GlobalLightPanel : public ImguiUnit
{
private:
    std::weak_ptr<omp::Light> m_LightRef;

public:
    explicit GlobalLightPanel(const std::shared_ptr<omp::Light>& inLight);

    virtual void renderUI(float DeltaTime);
};
}
