#pragma once

#include <memory>
#include <functional>
#include "ImguiUnit.h"
#include "imgui.h"

namespace omp
{

    class Camera;

    class ViewPort : public ImguiUnit
    {
        ImVec2 m_Size = { 100, 100};
        ImVec2 m_CursorPos = { 1, 1};
        bool m_Resized = false;
        ImTextureID m_ImageId;

        std::function<void(ImVec2)> m_Func;

        std::shared_ptr<omp::Camera> m_Camera;

    public:
        virtual void renderUi(float deltaTime) override;

        ImVec2 getSize() const { return m_Size; };
        ImVec2 getLocalCursorPos() { return m_CursorPos; }

        bool isResized() const { return m_Resized; }

        void setCamera(const std::shared_ptr<omp::Camera>& camera) { m_Camera = camera; };
        void setImageId(ImTextureID id) { m_ImageId = id; };
        void setMouseClickCallback(const std::function<void(ImVec2)> inFunc);
    };
} // omp

