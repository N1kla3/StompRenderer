#include "ScenePanel.h"
#include "imgui.h"

omp::ScenePanel::ScenePanel(const std::shared_ptr<EntityPanel>& entityPanel, const std::shared_ptr<omp::MaterialPanel>& inMatPanel)
        : ImguiUnit()
        , m_EntityUi(entityPanel)
        , m_MaterialPanel(inMatPanel)
{

}

void omp::ScenePanel::renderUi(float /*deltaTime*/)
{

    ImGui::Begin("Scene Panel");

    if (m_Scene)
    {
        if (ImGui::TreeNode("Scene items"))
        {
            for (auto& entity_ref : m_Scene->getEntities())
            {
                int32_t id = entity_ref->getId();
                int32_t current_id = m_Scene->getCurrentId();
                bool highlight_current = id == current_id;
                if (ImGui::Selectable(entity_ref->getName().c_str(), highlight_current))
                {
                    // on click selectable entity event
                    m_Scene->setCurrentId(entity_ref->getId());
                    m_EntityUi->setEntity(entity_ref.get());
                    //m_MaterialPanel->setMaterial(entity_ref->getModelInstance()->getMaterialInstance());
                }
                else if (highlight_current)
                {
                    m_EntityUi->setEntity(entity_ref.get());
                    //m_MaterialPanel->setMaterial(entity_ref->getModelInstance()->getMaterialInstance());
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

