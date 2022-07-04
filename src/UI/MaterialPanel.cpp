#include "MaterialPanel.h"
#include "imgui.h"
#include "Logs.h"

void omp::MaterialPanel::renderUI(float DeltaTime)
{
    ImGui::Begin("Material Panel");

    if (!m_Material.expired())
    {
        ImGui::BulletText("Textures:");
        for (auto& texture : m_Material.lock()->GetTextureData())
        {
            if (ImGui::TreeNode(texture.Texture->GetPath().c_str()))
            {
                if (ImGui::ImageButton(texture.Texture->GetTextureId(), {100, 100}))
                {
                    INFO(Rendering, "Pressed");
                }
                ImGui::TreePop();
            }
        }
    }
    else
    {
        ImGui::BulletText("No Material");
    }

    ImGui::End();
}

void omp::MaterialPanel::setMaterial(const std::shared_ptr<Material> &inMaterial)
{
    m_Material = inMaterial;
}
