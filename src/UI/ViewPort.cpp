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


    // RENDER RESIZING
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

    // KEYBOARD INPUT


    if (ImGui::IsKeyPressed(ImGuiKey_W))
    {
        m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_FORWARD, DeltaTime);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_A))
    {
        m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_LEFT, DeltaTime);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_S))
    {
        m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_BACK, DeltaTime);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_D))
    {
        m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_RIGHT, DeltaTime);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Q))
    {
        m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_UP, DeltaTime);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_E))
    {
        m_Camera->ProcessKeyboard(CAMERA_MOVEMENT::MOVE_DOWN, DeltaTime);
    }


    // MOUSE INPUT
    static bool should_reset_mouse = true;
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        static ImVec2 first_pos;
        if (should_reset_mouse)
        {
            first_pos = ImGui::GetMousePos();
            should_reset_mouse = false;
        }
        ImVec2 second = ImGui::GetMousePos();
        m_Camera->ProcessMouseMovement(second.x - first_pos.x, -(second.y - first_pos.y));
        first_pos = second;
    }
    else
    {
        should_reset_mouse = true;
    }

    ImGui::End();
}
