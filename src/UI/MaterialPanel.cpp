#include "MaterialPanel.h"
#include "imgui.h"
#include "Material.h"
#include "Logs.h"
#include "MaterialInstance.h"

void omp::MaterialPanel::renderUI(float DeltaTime)
{
    ImGui::Begin("Material Panel");

    if (!m_MaterialInstance.expired())
    {
        ImGui::BulletText("Textures:");
        for (auto& texture : m_MaterialInstance.lock()->m_StaticMaterial.lock()->GetTextureData())
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
        ImGui::DragFloat3("Ambient", &m_MaterialInstance.lock()->m_Ambient[0], 0.01f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Diffusive", &m_MaterialInstance.lock()->m_Diffusive[0], 0.01f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::DragFloat3("Specular", &m_MaterialInstance.lock()->m_Specular[0], 0.01f, 0.0f, 0.0f, "%.2f", 0);
    }
    else
    {
        ImGui::BulletText("No Material");
    }

    ImGui::End();
}

void omp::MaterialPanel::setMaterial(const std::shared_ptr<MaterialInstance> &inMaterial)
{
    m_MaterialInstance = inMaterial;
}
