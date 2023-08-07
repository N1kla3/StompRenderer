#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "imgui.h"

namespace omp
{

    class Camera;

    class ViewPort : public ImguiUnit
    {
        ImVec2 m_Size = { 100, 100};
        bool m_Resized = false;
        ImTextureID m_ImageId;

        std::shared_ptr<omp::Camera> m_Camera;

    public:
        virtual void renderUi(float deltaTime) override;

        ImVec2 getSize() const { return m_Size; };

        bool isResized() const { return m_Resized; }

        void setCamera(const std::shared_ptr<omp::Camera>& camera) { m_Camera = camera; };
        void setImageId(ImTextureID id) { m_ImageId = id; };
    };
} // omp

