#include "Model.h"

omp::Model::Model()
    : m_Name("NONE")
    , m_Transform(glm::mat4(1.f))
{

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
