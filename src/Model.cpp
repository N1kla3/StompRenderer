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

void omp::Model::SetMaterial(const std::shared_ptr<Material>& InMaterial)
{
    m_MaterialInstance = std::make_shared<MaterialInstance>(InMaterial);
}

void omp::Model::AddVertex(const omp::Vertex &InVertex)
{
    m_Vertices.emplace_back(std::move(InVertex));
}

void omp::Model::AddVertices(const std::vector<Vertex> &InVertices)
{
    m_Vertices = std::move(InVertices);
}

void omp::Model::AddIndex(uint32_t InIndex)
{
    m_Indices.push_back(InIndex);
}

void omp::Model::AddIndices(const std::vector<uint32_t> &InIndices)
{
    m_Indices = InIndices;
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

glm::vec3& omp::Model::GetPosition()
{
    return m_Translation;
}

glm::vec3& omp::Model::GetRotation()
{
    return m_Rotation;
}

glm::vec3& omp::Model::GetScale()
{
    return m_Scale;
}

glm::mat4 omp::Model::GetTransform() const
{
    glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));
    return glm::translate(glm::mat4(1.0f), m_Translation)
           * rotation
           * glm::scale(glm::mat4(1.0f), m_Scale);
}
