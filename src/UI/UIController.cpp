#include "UIController.h"
#include "Scene.h"
#include "UI/CameraPanel.h"
#include "UI/EntityPanel.h"
#include "Renderer.h"
#include "UI/ScenePanel.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "UI/MainLayer.h"

void omp::UIController::update(const omp::UIData& data, float deltaTime)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    // Should be first
    omp::MainLayer::update();
    omp::ScenePanel::update(data.scene);
    m_Viewport.updateUi(data.scene, data.scene->getCurrentCamera(), data.renderer->getViewportDescriptor());
    if (m_Viewport.isResized())
    {
        data.renderer->resizeViewport(static_cast<uint32_t>(m_Viewport.getSize().x), static_cast<uint32_t>(m_Viewport.getSize().y));
    }
    if (m_Viewport.isEntityClicked())
    {
        data.renderer->setClickedEntity(static_cast<uint32_t>(m_Viewport.getLocalCursorPos().x), static_cast<uint32_t>(m_Viewport.getLocalCursorPos().y));
    }
    if (data.scene)
    {
        omp::CameraPanel::update(data.scene->getCurrentCamera());
        omp::EntityPanel::update(data.scene->getCurrentEntity());
    }
    else
    {
        omp::CameraPanel::update(nullptr);
        omp::EntityPanel::update(nullptr);
    }

    ImGui::Render();
}
