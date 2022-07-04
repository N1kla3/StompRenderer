#include "ScenePanel.h"
#include "imgui.h"

omp::ScenePanel::ScenePanel(const std::shared_ptr<EntityPanel> &entityPanel)
    : ImguiUnit()
    , m_EntityUI(entityPanel)
{

}

void omp::ScenePanel::renderUI(float DeltaTime)
{

    ImGui::Begin("Scene Panel");

    if (!m_Scene.expired())
    {
        if (ImGui::TreeNode("Scene items"))
        {
            for (int16_t n = 0; n < m_Scene.lock()->GetModels().size(); n++)
            {
                if (ImGui::Selectable(m_Scene.lock()->GetModels()[n]->GetName().c_str(), m_SelectedIndex == n))
                {
                    m_SelectedIndex = n;
                    m_EntityUI->SetModel(m_Scene.lock()->GetModels()[m_SelectedIndex]);
                }
            }
            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::BulletText("No Scene");
    }

    ImGui::End();
}

