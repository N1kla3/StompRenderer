#pragma once
#include <memory>
#include <string>
#include "glm/vec4.hpp"

namespace omp{
    class Material;
class MaterialInstance
{
private:
    std::weak_ptr<Material> m_StaticMaterial;

    // vec4 because of glsl alignment, 4th element used for color strength
    glm::vec4 m_Ambient = {1.0f, 1.0f, 1.0f, 0};
    glm::vec4 m_Diffusive = {1.0f, 1.0f, 1.0f, 0};
    glm::vec4 m_Specular = {1.0f, 1.0f, 1.0f, 0};

public:
    MaterialInstance(const std::shared_ptr<Material>& materialCreateFrom);

    std::string GetShaderName() const;
    std::weak_ptr<Material> GetStaticMaterial() const { return m_StaticMaterial; }

    glm::vec4 GetAmbient() const { return m_Ambient; }
    glm::vec4 GetDiffusive() const { return m_Diffusive; }
    glm::vec4 GetSpecular() const { return m_Specular; }

    friend class MaterialPanel;
};
}
