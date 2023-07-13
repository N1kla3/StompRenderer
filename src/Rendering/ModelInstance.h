#pragma once

#include <string>
#include <memory>
#include "glm/vec3.hpp"
#include "MaterialInstance.h"
#include "glm/fwd.hpp"
#include "Model.h"

namespace omp
{
    class ModelInstance
    {
    private:
        std::string m_Name;

        glm::vec3 m_Translation = glm::vec3(1.f);
        glm::vec3 m_Rotation = glm::vec3(0.f);
        glm::vec3 m_Scale = glm::vec3(1.f);

        std::shared_ptr<MaterialInstance> m_MaterialInstance = nullptr;
        std::weak_ptr<Model> m_Model;
    public:
        ModelInstance();
        ModelInstance(const std::shared_ptr<omp::Model>& inModel);
        ModelInstance(const std::shared_ptr<omp::MaterialInstance>& inInstance);
        ModelInstance(const std::shared_ptr<omp::Model>& inModel, const std::shared_ptr<omp::MaterialInstance>& inInstance);

        std::string getName() const { return m_Name; }
        void setName(const std::string& inName) { m_Name = inName; }

        void setMaterialInstance(const std::shared_ptr<MaterialInstance>& inInstance);
        std::shared_ptr<MaterialInstance>& getMaterialInstance() { return m_MaterialInstance; }

        std::weak_ptr<omp::Model> getModel() const { return m_Model; }
        void setModel(const std::shared_ptr<omp::Model>& inModel);

        glm::mat4 getTransform() const;

        glm::vec3& getPosition();
        glm::vec3& getRotation();
        glm::vec3& getScale();
    };
}
