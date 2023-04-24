#pragma once

#include "glm/glm.hpp"


namespace omp
{
    struct Light
    {
        // GLSL will align to 4 words anyway(16byte)
        glm::vec4 position = {1.0f, 0.0f, 0.f, 0.f};

        glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 0.2};
        glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 0.4};
        glm::vec4 specular = {1.0f, 1.0f, 1.0f, 0.5};
    };

    struct DirectionalLight
    {
        glm::vec4 direction = {0.5f, 0.5f, 0.5f, 1.f};
    };
}