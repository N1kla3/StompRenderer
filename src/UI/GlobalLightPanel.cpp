#include "GlobalLightPanel.h"
#include "Light.h"
#include "imgui.h"

omp::GlobalLightPanel::GlobalLightPanel(const std::shared_ptr<omp::LightObject<omp::GlobalLight>>& inLight)
        : ImguiUnit()
        , m_LightRef(inLight)
{

}

void omp::GlobalLightPanel::renderUi(float /*deltaTime*/)
{
    ImGui::Begin("Light");

    /*if (!m_LightRef.expired())*/
    /*{*/
    /*    ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.5, 0.5, 0.5});*/
    /**/
    /*    omp::GlobalLight& temp_light = m_LightRef.lock()->getLight();*/
    /**/
    /*    ImGui::DragFloat3("Position", &temp_light.position_or_direction[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /**/
    /*    ImGui::DragFloat3("Ambient", &temp_light.ambient[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /*    ImGui::DragFloat("Ambient str", &temp_light.ambient[3], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /**/
    /*    ImGui::DragFloat3("Diffusive", &temp_light.diffuse[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /*    ImGui::DragFloat("Diffusive str", &temp_light.diffuse[3], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /**/
    /*    ImGui::DragFloat3("Specular", &temp_light.specular[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /*    ImGui::DragFloat("Specular str", &temp_light.specular[3], 0.1f, 0.0f, 0.0f, "%.2f", 0);*/
    /**/
    /*    ImGui::PopStyleColor(1);*/
    /*}*/
    /*else*/
    {
        ImGui::BulletText("No light source");
    }
    ImGui::End();
}

void omp::GlobalLightPanel::setLightRef(const std::shared_ptr<omp::LightObject<omp::GlobalLight>>& lightPtr)
{
    m_LightRef = lightPtr;
}

