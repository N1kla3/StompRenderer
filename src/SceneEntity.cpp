#include "SceneEntity.h"
#include "imgui.h"

omp::SceneEntity::SceneEntity()
    : SceneEntity("None", nullptr)
{

}

omp::SceneEntity::SceneEntity(const std::string& inName, const std::shared_ptr<omp::ModelInstance>& inModel)
    : m_Name(inName)
    , m_Model(inModel)
{
    // TODO id system
    static int32_t id = 1;
    m_Id = id++;
}

void omp::SceneEntity::draw()
{
    if (m_Model)
    {
        ImGui::Text("%s", m_Model->getName().c_str());
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0f, 0.5f, 0.5f, 0.5f});
        ImGui::DragFloat3("Position", &m_Model->getPosition()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.0f, 1.0f, 0.5f, 0.5f});
        ImGui::DragFloat3("Rotation", &m_Model->getRotation()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.6f, 0.5f, 1.0f, 0.5f});
        ImGui::DragFloat3("Scale", &m_Model->getScale()[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No Entity");
    }
}
