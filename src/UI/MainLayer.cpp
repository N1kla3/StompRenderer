#include "MainLayer.h"
#include "imgui.h"

void omp::MainLayer::renderUI()
{
    auto viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowClass window_class;
    window_class.ClassId = ImGui::GetID("XXX");
    //window_class2.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingSplitOther | ImGuiDockNodeFlags_NoDockingSplitMe;
    window_class.DockingAllowUnclassed = false;

    ImGuiWindowFlags host_window_flags = 0;
    host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
    host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    //-----------------Window2------------------------------
    ImGui::SetNextWindowClass(&window_class);

    ImGui::Begin("Window", NULL, host_window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockSpace = ImGui::GetID("dockSpace");
    ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f));

    ImGui::End();

}
