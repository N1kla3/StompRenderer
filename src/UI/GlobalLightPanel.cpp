#include "GlobalLightPanel.h"
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

    }
    else
    {

    }
    ImGui::End();
}
