#pragma once
#include <functional>
#include "ImguiUnit.h"
#include "imgui.h"
#include "glm/glm.hpp"
#include "ImGuizmo/ImGuizmo.h"

namespace omp
{

    class Camera;

    struct PickingInfo
    {
        int32_t id;
        glm::mat4 projection;
        glm::mat4 model;
    };

    class ViewPort : public ImguiUnit
    {
    private:
        PickingInfo m_Info;
        ImGuizmo::OPERATION m_Operation = ImGuizmo::TRANSLATE;

        ImVec2 m_Size = { 100, 100};
        ImVec2 m_CursorPos = { 1, 1};
        bool m_Resized = false;
        ImTextureID m_ImageId;

        std::function<void(ImVec2)> m_MouseClick;
        std::function<void(float[3])> m_TranslationChange;
        std::function<void(float[3])> m_RotationChange;
        std::function<void(float[3])> m_ScaleChange;

        omp::Camera* m_Camera;

    public:
        virtual void renderUi(float deltaTime) override;

        ImVec2 getSize() const { return m_Size; };
        ImVec2 getLocalCursorPos() { return m_CursorPos; }

        bool isResized() const { return m_Resized; }

        void setCamera(omp::Camera* camera) { m_Camera = camera; };
        void sendPickingData(PickingInfo info);
        void setImageId(ImTextureID id) { m_ImageId = id; };
        void setMouseClickCallback(const std::function<void(ImVec2)> inFunc);
        void setTranslationChangeCallback(const std::function<void(float[3])> inFunc);
        void setRotationChangeCallback(const std::function<void(float[3])> inFunc);
        void setScaleChangeCallback(const std::function<void(float[3])> inFunc);
    };
} // omp

