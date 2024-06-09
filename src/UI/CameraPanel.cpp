#include "CameraPanel.h"
#include "imgui.h"

omp::CameraPanel::CameraPanel(Camera* camera)
        : ImguiUnit()
        , m_Camera(camera)
{

}

void omp::CameraPanel::renderUi(float /*deltaTime*/)
{

    ImGui::Begin("Camera Panel");

    if (m_Camera)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.5, 0.5, 0.5});

        ImGui::DragFloat("Speed", &m_Camera->m_MovementSpeed, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("Sensitivity", &m_Camera->m_MouseSensitivity, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Position", &m_Camera->m_Position[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("View angle", &m_Camera->m_ViewAngle, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("Near clip distance", &m_Camera->m_NearClipping, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("Far clip distance", &m_Camera->m_FarClipping, 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No camera");
    }

    ImGui::End();
}

void omp::CameraPanel::setCamera(Camera* inCamera)
{
    m_Camera = inCamera;
}

