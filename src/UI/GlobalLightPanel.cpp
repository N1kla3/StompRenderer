#include "GlobalLightPanel.h"
#include "Light.h"
#include "imgui.h"

omp::GlobalLightPanel::GlobalLightPanel(const std::shared_ptr<omp::GlobalLight>& inLight)
        : ImguiUnit()
        , m_LightRef(inLight)
{

}

void omp::GlobalLightPanel::renderUi(float /*deltaTime*/)
{
    ImGui::Begin("Light");

    if (!m_LightRef.expired())
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.5, 0.5, 0.5});

        ImGui::DragFloat3("Position", &m_LightRef.lock()->position_or_direction[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::DragFloat3("Ambient", &m_LightRef.lock()->ambient[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("Ambient str", &m_LightRef.lock()->ambient[3], 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::DragFloat3("Diffusive", &m_LightRef.lock()->diffuse[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("Diffusive str", &m_LightRef.lock()->diffuse[3], 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::DragFloat3("Specular", &m_LightRef.lock()->specular[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat("Specular str", &m_LightRef.lock()->specular[3], 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No light source");
    }
    ImGui::End();
}
