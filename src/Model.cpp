#include "Model.h"

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
