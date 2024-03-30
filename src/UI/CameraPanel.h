#pragma once

#include "ImguiUnit.h"
#include "Camera.h"

namespace omp
{
    class CameraPanel : public ImguiUnit
    {
    public:
        CameraPanel(Camera* camera);
        virtual void renderUi(float deltaTime) override;

    private:
        Camera* m_Camera;
    };
}
