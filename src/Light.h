#pragma once
#include "glm/glm.hpp"


namespace omp{
struct Light
{
    glm::vec3 m_Position = {1.0f, 0.0f, 0.f};
    glm::vec3 m_Color = {1.0f, 1.0f, 1.0f};
};
}