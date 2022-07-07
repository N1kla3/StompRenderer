#pragma once
#include "glm/glm.hpp"


namespace omp{
class Light
{
private:
    glm::vec3 m_Direction;
    float m_Intencity;
    glm::vec3 m_Color;

public:


    glm::vec3 GetDirection() const{ return m_Direction; }
    float GetIntensity() const{ return m_Intencity; }
    glm::vec3 GetColor() const { return m_Color; }
};
}