#include "EntityPanel.h"
#include "Rendering/Material.h"
#include "imgui.h"

void omp::EntityPanel::setEntity(const std::shared_ptr<IDrawable>& inEntity)
{
    m_Entity = inEntity;
}

void omp::EntityPanel::renderUi(float deltaTime)
{

    ImGui::Begin("Entity Panel");

    if (m_Entity.lock())
    {
        m_Entity.lock()->draw();
    }

    ImGui::End();
}
