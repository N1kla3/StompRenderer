#include "ViewPort.h"
#include "imgui.h"

void omp::ViewPort::renderUI()
{
    auto viewport = ImGui::GetMainViewport();
    auto viewport_size = viewport->WorkSize;

    ImGuiWindowClass Viewort;
    Viewort.ClassId = ImGui::GetID("Window");
    Viewort.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_NoSplit | ImGuiDockNodeFlags_PassthruCentralNode;
    Viewort.DockingAllowUnclassed = true;

    ImGui::SetNextWindowClass(&Viewort);
    ImGui::Begin("Viewport", NULL, 0);

    ImVec2 new_size = ImGui::GetWindowSize();
    float monitor_x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
    if (monitor_x > viewport_size.x)
    {
        new_size.x -= monitor_x - viewport_size.x;
    }
    float monitor_y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
    if (monitor_y > viewport_size.y)
    {
        new_size.y -= monitor_y - viewport_size.y;
    }

    ImVec2 new_pos = ImGui::GetWindowPos();
    if (ImGui::GetWindowPos().x < 0)
    {
        new_size.x += ImGui::GetWindowPos().x;
        new_pos.x = 0;
    }
    if (ImGui::GetWindowPos().y < 0)
    {
        new_size.y += ImGui::GetWindowPos().y;
        new_pos.y = 0;
    }

    new_size.y -= 19;
    m_Size = new_size;
    new_pos.y += 19;
    m_Offset = new_pos;

    if (m_Size.x != ImGui::GetWindowSize().x || m_Offset.x != ImGui::GetWindowPos().x
        || m_Size.y != ImGui::GetWindowSize().y || m_Offset.y != ImGui::GetWindowPos().y)
    {
        m_Resized = true;
    }
    else
    {
        m_Resized = false;
    }
    ImGui::End();

}
