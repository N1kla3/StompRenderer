#include "EntityPanel.h"
#include "Rendering/Material.h"
#include "imgui.h"

void omp::EntityPanel::setEntity(IDrawable* inEntity)
{
    m_Entity = inEntity;
}

void omp::EntityPanel::renderUi(float /*deltaTime*/)
{

    ImGui::Begin("Entity Panel");

    if (m_Entity)
    {
        m_Entity->draw();
    }

    ImGui::End();
}
