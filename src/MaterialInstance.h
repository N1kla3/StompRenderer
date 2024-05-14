#pragma once

#include <memory>
#include <string>
#include "glm/vec4.hpp"

namespace omp
{
    class Material;

    class MaterialInstance
    {
    private:
        std::shared_ptr<Material> m_StaticMaterial;

        // vec4 because of glsl alignment, 4th element used for color strength
        glm::vec4 m_Ambient = {1.0f, 1.0f, 1.0f, 0};
        glm::vec4 m_Diffusive = {1.0f, 1.0f, 1.0f, 0};
        glm::vec4 m_Specular = {1.0f, 1.0f, 1.0f, 0};

    public:
        MaterialInstance(const std::shared_ptr<Material>& materialCreateFrom);

        std::weak_ptr<Material> getStaticMaterial() const { return m_StaticMaterial; }

        void setAmbient(glm::vec4 new_ambient){ m_Ambient = new_ambient; }
        glm::vec4 getAmbient() const { return m_Ambient; }

        void setDiffusive(glm::vec4 new_diffusive){ m_Ambient = new_diffusive; }
        glm::vec4 getDiffusive() const { return m_Diffusive; }

        void setSpecular(glm::vec4 new_specular){ m_Ambient = new_specular; }
        glm::vec4 getSpecular() const { return m_Specular; }

        friend class MaterialPanel;
    };
}
