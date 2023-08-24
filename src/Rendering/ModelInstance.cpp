#include "ModelInstance.h"
#include "glm/gtx/quaternion.hpp"

glm::vec3& omp::ModelInstance::getPosition()
{
    return m_Translation;
}

glm::vec3& omp::ModelInstance::getRotation()
{
    return m_Rotation;
}

glm::vec3& omp::ModelInstance::getScale()
{
    return m_Scale;
}

glm::mat4 omp::ModelInstance::getTransform() const
{
    glm::vec3 rad_rot = glm::radians(m_Rotation);
    glm::mat4 rotation = glm::toMat4(glm::quat(rad_rot));
    return glm::translate(glm::mat4(1.0f), m_Translation)
           * rotation
           * glm::scale(glm::mat4(1.0f), m_Scale);
}

omp::ModelInstance::ModelInstance()
{

}

omp::ModelInstance::ModelInstance(const std::shared_ptr<omp::Model>& inModel)
    : m_Model(inModel)
{

}

omp::ModelInstance::ModelInstance(const std::shared_ptr<omp::MaterialInstance>& inInstance)
    : m_MaterialInstance(inInstance)
{

}

omp::ModelInstance::ModelInstance(
        const std::shared_ptr<omp::Model>& inModel,
        const std::shared_ptr<omp::MaterialInstance>& inInstance)
        : m_Model(inModel)
        , m_MaterialInstance(inInstance)
{

}
omp::ModelInstance::ModelInstance(
        const std::shared_ptr<omp::Model>& inModel,
        const std::shared_ptr<omp::Material>& inMat)
        : m_Model(inModel)
        , m_MaterialInstance(std::make_shared<omp::MaterialInstance>(inMat))
{

}

void omp::ModelInstance::setMaterialInstance(const std::shared_ptr<MaterialInstance>& inInstance)
{
    m_MaterialInstance = inInstance;
}

void omp::ModelInstance::setModel(const std::shared_ptr<omp::Model>& inModel)
{
    m_Model = inModel;
}
