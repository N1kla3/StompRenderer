#include "ModelStatics.h"
#include "tiny_obj_loader.h"

void omp::ModelImporter::loadModel(omp::Model* model, const std::string& inPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inPath.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<omp::Vertex, uint32_t> unique_vertices;

    unique_vertices.clear();

    for (const auto& shape: shapes)
    {
        for (const auto& index: shape.mesh.indices)
        {
            omp::Vertex vertex{};

            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1 - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
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
}
