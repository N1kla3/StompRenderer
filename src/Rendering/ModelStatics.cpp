#include "ModelStatics.h"
#include "tiny_obj_loader.h"
#include "Core/Profiling.h"

bool omp::ModelImporter::loadModel(omp::Model* model, const std::string& inPath)
{
    OMP_STAT_SCOPE("LoadModel");

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inPath.c_str()))
    {
        ERROR(LogIO, "tinyobj::LoadObj Failed with output {} {}", warn, err);
        return false;
    }

    std::unordered_map<omp::Vertex, uint32_t> unique_vertices;

    unique_vertices.clear();

    for (const auto& shape: shapes)
    {
        for (const auto& index: shape.mesh.indices)
        {
            omp::Vertex vertex{};

            size_t vertex_index = index.vertex_index;
            size_t texcoord_index = index.texcoord_index;
            size_t normal_index = index.normal_index;
            vertex.pos = {
                    attrib.vertices[3U * vertex_index + 0U],
                    attrib.vertices[3U * vertex_index + 1U],
                    attrib.vertices[3U * vertex_index + 2U]
            };
            vertex.tex_coord = {
                    attrib.texcoords[2U * texcoord_index + 0U],
                    1 - attrib.texcoords[2U * texcoord_index + 1U]
            };
            vertex.color = {
                    attrib.colors[3U * vertex_index + 0U],
                    attrib.colors[3U * vertex_index + 1U],
                    attrib.colors[3U * vertex_index + 2U]
            };

            vertex.normal = {
                    attrib.normals[3U * normal_index + 0U],
                    attrib.normals[3U * normal_index + 1U],
                    attrib.normals[3U * normal_index + 2U]
            };
            // TODO incorrect amount

            if (unique_vertices.count(vertex) == 0)
            {
                unique_vertices[vertex] = static_cast<uint32_t>(model->getVertices().size());
                model->addVertex(vertex);
            }

            model->addIndex(unique_vertices[vertex]);
        }
    }
    return true;
}
