#pragma once

#include <vulkan/vulkan.h>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/glm.hpp"
#include "Material.h"
#include "MaterialInstance.h"
#include <array>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>
#include <vector>

namespace omp
{
    class Model;

    struct Vertex;
    struct ModelPushConstant;
}

struct omp::ModelPushConstant
{
    glm::mat4 model;

    glm::vec4 ambient;
    glm::vec4 diffusive;
    glm::vec4 specular;
};

struct omp::Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
    glm::vec3 normal;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

        attribute_descriptions[3].binding = 0;
        attribute_descriptions[3].location = 3;
        attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[3].offset = offsetof(Vertex, normal);

        return attribute_descriptions;
    }

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && tex_coord == other.tex_coord && normal == other.normal;
    }
};

namespace std
{
    template<>
    struct hash<omp::Vertex>
    {
        size_t operator()(omp::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                     (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.tex_coord) << 1);//TODO add normal
        }
    };
}

class omp::Model
{
public:
    // Lifecycle //
    // ========= //
    Model();
    ~Model() = default;

private:
    // State //
    // ===== //
    std::string m_Name;

    glm::vec3 m_Translation;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;

    std::vector<Vertex> m_Vertices;

    std::vector<uint32_t> m_Indices;

    std::shared_ptr<MaterialInstance> m_MaterialInstance;

public:
    // Methods //
    // ======= //
    //void RotateModel(float angle, const glm::vec3& rotationAxis);
    //void MoveModel(const glm::vec3& translation);
    //void ScaleModel(const glm::vec3& scale);
    //void SetTransform();

    void setName(const std::string& inName) { m_Name = inName; }

    void setMaterial(const std::shared_ptr<Material>& inMaterial);
    void addVertex(const omp::Vertex& inVertex);
    void addVertices(const std::vector<Vertex>& inVertices);
    void addIndex(uint32_t inIndex);
    void addIndices(const std::vector<uint32_t>& inIndices);

    const std::string& getName() const { return m_Name; }

    std::shared_ptr<MaterialInstance>& getMaterialInstance() { return m_MaterialInstance; }

    glm::mat4 getTransform() const;

    const std::vector<Vertex>& getVertices() const { return m_Vertices; }

    const std::vector<uint32_t>& getIndices() const { return m_Indices; }

    glm::vec3& getPosition();
    glm::vec3& getRotation();
    glm::vec3& getScale();
};


