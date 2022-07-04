#include "ViewPort.h"
#include "Camera.h"
#include "imgui.h"

void omp::ViewPort::renderUI(float DeltaTime)
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

    if (!m_Camera || !ImGui::IsWindowFocused())
    {
        ImGui::End();
        return;
    }

    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
    const ImGuiKey key_first = 0;
    for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++)
    {
        if (funcs::IsLegacyNativeDupe(key)) continue;
        if (ImGui::IsKeyPressed(key))
        {
            if (key == ImGuiKey_W)
            {
                m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_FORWARD, DeltaTime);
            }
            if (key == ImGuiKey_A)
            {
                m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_LEFT, DeltaTime);
            }
            if (key == ImGuiKey_S)
            {
                m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_BACK, DeltaTime);
            }
            if (key == ImGuiKey_D)
            {
                m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_RIGHT, DeltaTime);
            }
            if (key == ImGuiKey_Q)
            {
                m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_UP, DeltaTime);
            }
            if (key == ImGuiKey_E)
            {
                m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_DOWN, DeltaTime);
            }
        }
    }

    ImGui::End();
}
