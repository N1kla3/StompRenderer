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

        float constant;
        float linear;
        float quadratic;

        glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 0.2};
        glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 0.4};
        glm::vec4 specular = {1.0f, 1.0f, 1.0f, 0.5};
    };

    struct SpotLight
    {
        glm::vec4 position;
        glm::vec4 direction;
        float cut_off;
        float outer_cutoff;

        glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 0.2};
        glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 0.4};
        glm::vec4 specular = {1.0f, 1.0f, 1.0f, 0.5};
    };
}