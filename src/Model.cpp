#include "Model.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

omp::Model::Model()
        : m_Name("NONE")
        , m_Translation(glm::vec3{1.f})
        , m_Rotation(glm::vec3{0.f})
        , m_Scale(glm::vec3{1.f})
{

}

void omp::Model::setMaterial(const std::shared_ptr<Material>& inMaterial)
{
    m_MaterialInstance = std::make_shared<MaterialInstance>(inMaterial);
}

void omp::Model::addVertex(const omp::Vertex& inVertex)
{
    m_Vertices.emplace_back(std::move(inVertex));
}

void omp::Model::addVertices(const std::vector<Vertex>& inVertices)
{
    m_Vertices = std::move(inVertices);
}

void omp::Model::addIndex(uint32_t inIndex)
{
    m_Indices.push_back(inIndex);
}

void omp::Model::addIndices(const std::vector<uint32_t>& inIndices)
{
    m_Indices = inIndices;
}

/*
void omp::Model::RotateModel(float angle, const glm::vec3& rotationAxis)
{
    m_Transform = glm::rotate(m_Transform, angle, rotationAxis);
}

void omp::Model::MoveModel(const glm::vec3& translation)
{
    m_Transform = glm::translate(m_Transform, translation);
}

void omp::Model::ScaleModel(const glm::vec3& scale)
{
    m_Transform = glm::scale(m_Transform, scale);
}

void omp::Model::SetTransform()
{

}
 */

glm::vec3& omp::Model::getPosition()
{
    return m_Translation;
}

glm::vec3& omp::Model::getRotation()
{
    return m_Rotation;
}

glm::vec3& omp::Model::getScale()
{
    return m_Scale;
}

glm::mat4 omp::Model::getTransform() const
{
    glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));
    return glm::translate(glm::mat4(1.0f), m_Translation)
           * rotation
           * glm::scale(glm::mat4(1.0f), m_Scale);
}
