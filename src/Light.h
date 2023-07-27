#pragma once
#include "glm/glm.hpp"


namespace omp
{
    struct GlobalLight
    {
        // GLSL will align to 4 words anyway(16byte)
        // if directional light, then W = 1.0
        glm::vec4 position_or_direction = {1.0f, 0.0f, 0.f, 0.f};

        glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 0.2};
        glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 0.4};
        glm::vec4 specular = {1.0f, 1.0f, 1.0f, 0.5};
    };

    struct PointLight
    {
        glm::vec4 position;

        glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 0.2};
        glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 0.4};
        glm::vec4 specular = {1.0f, 1.0f, 1.0f, 0.5};

        float constant = 1.f;
        float linear = 0.07f;
        float quadratic = 0.017f;
        float unused;
    };

    struct SpotLight
    {
        glm::vec4 position;

        glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 0.2};
        glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 0.4};
        glm::vec4 specular = {1.0f, 1.0f, 1.0f, 0.5};

        glm::vec4 direction;

        float cut_off = 0.91f;
        float outer_cutoff = 0.82f;

        float constant = 1.f;
        float linear = 0.07f;

        glm::vec3 unu;
        float quadratic = 0.017f;
    };
}