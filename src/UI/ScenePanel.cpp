#include "ScenePanel.h"
#include "imgui.h"

omp::ScenePanel::ScenePanel(const std::shared_ptr<EntityPanel>& entityPanel)
        : ImguiUnit()
        , m_EntityUi(entityPanel)
{

}

void omp::ScenePanel::renderUi(float deltaTime)
{

    ImGui::Begin("Scene Panel");

    if (!m_Scene.expired())
    {
        if (ImGui::TreeNode("Scene items"))
        {
            for (int16_t n = 0; n < m_Scene.lock()->getModels().size(); n++)
            {
                if (ImGui::Selectable(m_Scene.lock()->getModels()[n]->getName().c_str(), m_SelectedIndex == n))
                {
                    m_SelectedIndex = n;
                    m_EntityUi->setModel(m_Scene.lock()->getModels()[m_SelectedIndex]);
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

