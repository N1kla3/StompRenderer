#pragma once
#include "glm/glm.hpp"


namespace omp{
class Light
{
public:
    // GLSL will align to 4 words anyway(16byte)
    glm::vec4 m_Position = {1.0f, 0.0f, 0.f, 0.f};

    glm::vec4 m_Ambient = {1.0f, 1.0f, 1.0f, 0};
    glm::vec4 m_Diffuse = {1.0f, 1.0f, 1.0f, 0};
    glm::vec4 m_Specular = {1.0f, 1.0f, 1.0f, 0};


};
}