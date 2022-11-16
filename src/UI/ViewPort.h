#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "imgui.h"

namespace omp
{

    class Camera;

    class ViewPort : public ImguiUnit
    {
        ImVec2 m_Size;
        ImVec2 m_Offset;
        bool m_Resized = false;

        std::shared_ptr<omp::Camera> m_Camera;

    public:
        virtual void renderUi(float deltaTime) override;

        ImVec2 getSize() const { return m_Size; };

        ImVec2 getOffset() const { return m_Offset; }

        bool isResized() const { return m_Resized; }

        void setCamera(const std::shared_ptr<omp::Camera>& camera) { m_Camera = camera; };
    };
} // omp

