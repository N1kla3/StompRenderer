#pragma once
#include "imgui.h"
#include "glm/glm.hpp"
#include "ImGuizmo/ImGuizmo.h"

namespace omp
{
    class Camera;
    class Scene;

    struct PickingInfo
    {
        bool isPicked;
        glm::mat4 projection;
        glm::mat4 model;
    };

    class ViewPort
    {
    private:
        PickingInfo m_Info;
        ImGuizmo::OPERATION m_Operation = ImGuizmo::TRANSLATE;

        ImVec2 m_Size = { 100, 100};
        ImVec2 m_CursorPos = { 1, 1};
        bool m_Resized = false;
        bool m_ClickedEntity = false;

    public:
        void updateUi(omp::Scene* scene, omp::Camera* camera, ImTextureID viewportImage);

        ImVec2 getSize() const { return m_Size; };
        ImVec2 getLocalCursorPos() const { return m_CursorPos; }

        bool isResized() const { return m_Resized; }
        bool isEntityClicked() const { return m_ClickedEntity; }
    };
} // omp

