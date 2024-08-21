#include "ScenePanel.h"
#include "Scene.h"
#include "imgui.h"

void omp::ScenePanel::update(omp::Scene* scene)
{

    ImGui::Begin("Scene Panel");

    if (scene)
    {
        if (ImGui::TreeNode("Scene items"))
        {
            for (auto& entity_ref : scene->getEntities())
            {
                int32_t id = entity_ref->getId();
                int32_t current_id = scene->getCurrentId();
                bool highlight_current = id == current_id;
                if (ImGui::Selectable(entity_ref->getName().c_str(), highlight_current))
                {
                    // on click selectable entity event
                    scene->setCurrentId(entity_ref->getId());
                }

            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Camera Items"))
        {
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Light Items"))
        {
            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::BulletText("No Scene");
    }

    ImGui::End();
}

