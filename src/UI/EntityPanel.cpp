#include "EntityPanel.h"
#include "Material.h"
#include "imgui.h"

omp::EntityPanel::EntityPanel(const std::shared_ptr<MaterialPanel>& materialPanel)
{
    m_MaterialPanelRef = materialPanel;
}

void omp::EntityPanel::renderUi(float deltaTime)
{
    ImGui::Begin("Entity Panel");

    if (!m_Model.expired())
    {
        ImGui::Text("%s", m_Model.lock()->getName().c_str());
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.5, 0.5, 0.5});
        ImGui::DragFloat3("Position", &m_Model.lock()->getPosition()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.0, 1.0, 0.5, 0.5});
        ImGui::DragFloat3("Rotation", &m_Model.lock()->getRotation()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.6, 0.5, 1.0, 0.5});
        ImGui::DragFloat3("Scale", &m_Model.lock()->getScale()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No Entity");
    }

    ImGui::End();
}

void omp::EntityPanel::setModel(const std::shared_ptr<omp::Model>& model)
{
    m_Model = model;
    auto mat_panel = m_MaterialPanelRef.lock();
    if (mat_panel)
    {
        mat_panel->setMaterial(model->getMaterialInstance());
    }
}
