#pragma once

#include "ImguiUnit.h"
#include "Camera.h"

namespace omp
{
    class CameraPanel : public ImguiUnit
    {
    public:
        CameraPanel() = default;
        CameraPanel(Camera* camera);
        virtual void renderUi(float deltaTime) override;
        void setCamera(Camera* inCamera);

    private:
        Camera* m_Camera = nullptr;
    };
}
