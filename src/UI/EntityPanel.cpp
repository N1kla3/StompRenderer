#include "EntityPanel.h"
#include "imgui.h"

void omp::EntityPanel::update(IDrawable* entity)
{
    ImGui::Begin("Entity Panel");

    if (entity)
    {
        entity->draw();
    }

    ImGui::End();
}
