#include "ViewPort.h"
#include "Camera.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "ImGuizmo/ImGuizmo.h"
#include "Logs.h"

void omp::ViewPort::renderUi(float /*deltaTime*/)
{
    ImGuiWindowClass window_class;
    window_class.ClassId = ImGui::GetID("Window");
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_NoSplit |
                                            ImGuiDockNodeFlags_PassthruCentralNode;
    window_class.DockingAllowUnclassed = true;

    ImGui::SetNextWindowClass(&window_class);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", NULL, 0);
    ImGui::PopStyleVar(3);


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
    ImVec2 viewport_cursor = ImGui::GetMousePos();
    auto window_pos = ImGui::GetWindowPos();
    auto content_min= ImGui::GetWindowContentRegionMin();
    auto content_max = ImGui::GetWindowContentRegionMax();

    viewport_cursor.x = viewport_cursor.x - window_pos.x - content_min.x;
    viewport_cursor.y = viewport_cursor.y - window_pos.y - content_min.y;

    if (m_Camera)
    {
        ImGui::Image(m_ImageId, m_Size);
    }

    if (m_Info.id != -1)
    {
        //ImGuizmo::Enable(true);

        ImGuizmo::SetOrthographic(true);
        ImGuizmo::SetDrawlist();
        float w = ImGui::GetWindowWidth();
        float h = ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, w, h);
        ImGuizmo::Manipulate(glm::value_ptr(m_Camera->getViewMatrix()), glm::value_ptr(m_Info.projection),
                             m_Operation, ImGuizmo::LOCAL, glm::value_ptr(m_Info.model));

        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(value_ptr(m_Info.model), matrixTranslation, matrixRotation, matrixScale);

        if (ImGuizmo::IsUsingAny())
        {
            m_TranslationChange(matrixTranslation);
            m_RotationChange(matrixRotation);
            m_ScaleChange(matrixScale);
        }
    }


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
    if (ImGui::IsKeyDown(ImGuiKey_E))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_UP);
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q))
    {
        m_Camera->processKeyboard(ECameraMovement::MOVE_DOWN);
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_R))
    {
        m_Operation = ImGuizmo::TRANSLATE;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_T))
    {
        m_Operation = ImGuizmo::ROTATE;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Y))
    {
        m_Operation = ImGuizmo::SCALE;
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

    if (viewport_cursor.x > 0 && viewport_cursor.x < content_max.x
        && viewport_cursor.y > 0 && viewport_cursor.y < content_max.y
        && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
    {
        m_CursorPos = viewport_cursor;
        m_MouseClick(m_CursorPos);
    }

    ImGui::End();
}


void omp::ViewPort::setMouseClickCallback(const std::function<void(ImVec2)> inFunc)
{
    m_MouseClick = inFunc;
}

void omp::ViewPort::sendPickingData(PickingInfo info)
{
    m_Info = info;
}

void omp::ViewPort::setTranslationChangeCallback(const std::function<void(float*)> inFunc)
{
    m_TranslationChange = inFunc;
}

void omp::ViewPort::setRotationChangeCallback(const std::function<void(float*)> inFunc)
{
    m_RotationChange = inFunc;
}

void omp::ViewPort::setScaleChangeCallback(const std::function<void(float*)> inFunc)
{
    m_ScaleChange = inFunc;
}
