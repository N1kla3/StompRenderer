#include "GlobalLightPanel.h"
#include "Light.h"
#include "imgui.h"

omp::GlobalLightPanel::GlobalLightPanel(const std::shared_ptr<omp::Light> &inLight)
    : ImguiUnit()
    , m_LightRef(inLight)
{

}

void omp::GlobalLightPanel::renderUI(float DeltaTime)
{
    ImGui::Begin("Light");

    if (!m_LightRef.expired())
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.5, 0.5, 0.5});

        ImGui::DragFloat3("Position", &m_LightRef.lock()->m_Position[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Color", &m_LightRef.lock()->m_Color[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Ambient", &m_LightRef.lock()->m_Ambient[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Diffusive", &m_LightRef.lock()->m_Diffuse[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Specular", &m_LightRef.lock()->m_Specular[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No light source");
    }
    ImGui::End();
}
