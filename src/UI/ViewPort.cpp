#include "ViewPort.h"
#include "Camera.h"
#include "Scene.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "ImGuizmo/ImGuizmo.h"
#include "stomp_imconfig.h"

void omp::ViewPort::updateUi(omp::Scene* scene, omp::Camera* camera, VkDescriptorSet viewportImage)
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


    ImVec2 new_size = ImGui::GetContentRegionAvail();
    m_Resized = false;
    if (new_size.x != m_Size.x || new_size.y != m_Size.y)
    {
        m_Size = new_size;
        m_Resized = true;
    }

    ImVec2 viewport_cursor = ImGui::GetMousePos();
    auto window_pos = ImGui::GetWindowPos();
    auto content_min= ImGui::GetWindowContentRegionMin();
    auto content_max = ImGui::GetWindowContentRegionMax();

    viewport_cursor.x = viewport_cursor.x - window_pos.x - content_min.x;
    viewport_cursor.y = viewport_cursor.y - window_pos.y - content_min.y;

    m_ClickedEntity = false;

    if (camera)
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(viewportImage), m_Size);
    }

    if (m_Info.isPicked)
    {
        ImGuizmo::Enable(true);

        ImGuizmo::SetOrthographic(true);
        ImGuizmo::SetDrawlist();
        float w = ImGui::GetWindowWidth();
        float h = ImGui::GetWindowHeight();
        auto entity = scene->getCurrentEntity();
        m_Info.model = entity->getModelInstance()->getTransform();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, w, h);
        bool manipulated = ImGuizmo::Manipulate(glm::value_ptr(camera->getViewMatrix()), glm::value_ptr(m_Info.projection),
                             m_Operation, ImGuizmo::LOCAL, glm::value_ptr(m_Info.model));

        glm::vec3 matrixTranslation, matrixRotation, matrixScale;
        ImGuizmo::DecomposeMatrixToComponents(value_ptr(m_Info.model), &matrixTranslation[0], &matrixRotation[0], &matrixScale[0]);

        if (ImGuizmo::IsUsingAny())
        {
            ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
            if (manipulated)
            {
                entity->setTranslation(matrixTranslation);
                entity->setRotation(matrixRotation);
                entity->setScale(matrixScale);
            }
        }
        else
        {

            ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
        }
    }


    if (!camera || !ImGui::IsWindowFocused())
    {
        ImGui::End();
        return;
    }

    // KEYBOARD INPUT


    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        camera->processKeyboard(ECameraMovement::MOVE_FORWARD);
    }
    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        camera->processKeyboard(ECameraMovement::MOVE_LEFT);
    }
    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        camera->processKeyboard(ECameraMovement::MOVE_BACK);
    }
    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        camera->processKeyboard(ECameraMovement::MOVE_RIGHT);
    }
    if (ImGui::IsKeyDown(ImGuiKey_E))
    {
        camera->processKeyboard(ECameraMovement::MOVE_UP);
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q))
    {
        camera->processKeyboard(ECameraMovement::MOVE_DOWN);
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
        camera->processMouseMovement(second.x - first_pos.x, -(second.y - first_pos.y));
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
        m_ClickedEntity = true;
        m_CursorPos = viewport_cursor;
        m_Info.isPicked = true;
        m_Info.projection = glm::perspective(glm::radians(camera->getViewAngle()),
                                             m_Size.x / m_Size.y,
                                             camera->getNearClipping(), camera->getFarClipping());
    }

    ImGui::End();
}

