#include "ModelStatics.h"
#include "tiny_obj_loader.h"

void omp::ModelManager::loadModel(const std::string& inPath)
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

    std::shared_ptr<omp::Model> loaded_model = std::make_shared<omp::Model>();

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
                unique_vertices[vertex] = static_cast<uint32_t>(loaded_model->getVertices().size());
                loaded_model->addVertex(vertex);
            }

            loaded_model->addIndex(unique_vertices[vertex]);
        }
    }
    loaded_model->setName(inPath);
    m_Models.insert({inPath, loaded_model});
}

std::shared_ptr<omp::Model> omp::ModelManager::getModel(const std::string& inPath) const
{
    if (m_Models.find(inPath) != m_Models.end())
    {
        return m_Models.at(inPath);
    }
    return nullptr;
}

std::shared_ptr<omp::Model> omp::ModelManager::forceGetModel(const std::string& inPath)
{
    if (m_Models.find(inPath) == m_Models.end())
    {
        loadModel(inPath);
    }
    return m_Models.at(inPath);
}

std::shared_ptr<omp::ModelInstance> omp::ModelManager::createInstanceFrom(const std::string& inPath)
{
    auto ptr = getModel(inPath);
    if (ptr.get())
    {
        auto instance = std::make_shared<omp::ModelInstance>(ptr);
        return instance;
    }
    return nullptr;
}
