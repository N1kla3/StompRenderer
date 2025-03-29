// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

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
