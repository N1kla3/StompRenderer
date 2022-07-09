#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Camera.h"

namespace omp{
class CameraPanel : public ImguiUnit
{
public:
    CameraPanel(const std::shared_ptr<Camera>& camera);
    virtual void renderUI(float DeltaTime) override;

private:
    std::weak_ptr<Camera> m_Camera;
};
}