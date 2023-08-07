#include "ViewPort.h"
#include "Camera.h"
#include "imgui.h"

void omp::ViewPort::renderUi(float deltaTime)
{
    auto viewport = ImGui::GetMainViewport();
    auto viewport_size = viewport->WorkSize;

    ImGuiWindowClass window_class;
    window_class.ClassId = ImGui::GetID("Window");
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_NoSplit |
                                            ImGuiDockNodeFlags_PassthruCentralNode;
    window_class.DockingAllowUnclassed = true;

    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin("Viewport", NULL, 0);


    auto new_size = ImGui::GetContentRegionAvail();

    if (new_size.x != m_Size.x
        || new_size.y != m_Size.y)
    {
        m_Resized = true;
        m_Size = new_size;
    }
    else
    {
        m_Resized = false;
    }

    ImGui::Image(m_ImageId, m_Size);


    if (!m_Camera || !ImGui::IsWindowFocused())
    {
        ImGui::End();
        return;
    }

    // KEYBOARD INPUT


    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_FORWARD);
    }
    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_LEFT);
    }
    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_BACK);
    }
    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_RIGHT);
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_UP);
    }
    if (ImGui::IsKeyDown(ImGuiKey_E))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_DOWN);
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
        m_Camera->processMouseMovement(second.x - first_pos.x, -(second.y - first_pos.y));
        first_pos = second;
    }
    else
    {
        should_reset_mouse = true;
    }

    ImGui::End();
}
