#pragma once

#include <vulkan/vulkan.h>
#include "Math/GlmHash.h"
#include "Material.h"
#include "MaterialInstance.h"
#include <array>
#include "IO/SerializableObject.h"
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

    glm::vec4 ambient{1};
    glm::vec4 diffusive{2};
    glm::vec4 specular{3};

    uint32_t id = 4;

    ModelPushConstant(const glm::mat4& inModel, const glm::vec4& inAmbient, const glm::vec4& inDiffusive, const glm::vec4& inSpecular, uint32_t inId)
        : model(inModel)
        , ambient(inAmbient)
        , diffusive(inDiffusive)
        , specular(inSpecular)
        , id(inId)
    {

    }
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

class omp::Model : public SerializableObject
{
public:
    // Lifecycle //
    // ========= //
    Model();
    Model(const std::string& path);
    Model(const Model& inModel) = delete;
    Model(Model&& inModel) = delete;
    ~Model();

    virtual void serialize(JsonParser<>& parser) override;
    virtual void deserialize(JsonParser<>& parser) override;

private:
    void loadVertexToMemory();
    void loadIndexToMemory();
    void tryClear();

    void addVertex(const omp::Vertex& inVertex);
    void addVertices(const std::vector<Vertex>& inVertices);
    void addIndex(uint32_t inIndex);
    void addIndices(const std::vector<uint32_t>& inIndices);

    // State //
    // ===== //
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;

    std::string m_Name;
    std::string m_Path;

    std::weak_ptr<omp::VulkanContext> m_Context{};

    VkBuffer m_IndexBuffer;
    VkDeviceMemory m_IndexMemory;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexMemory;

    bool m_Loaded = false;

public:
    // Methods //
    // ======= //
    void setName(const std::string& inName) { m_Name = inName; }
    void setPath(const std::string& inPath) { m_Path = inPath; }

    void loadToGpuMemory(const std::shared_ptr<omp::VulkanContext>& context, bool forceUpdate);

    const std::string& getName() const { return m_Name; }
    const std::string& getPath() const { return m_Path; }
    bool isLoaded() const { return m_Loaded; }

    const std::vector<Vertex>& getVertices() const { return m_Vertices; }

    const std::vector<uint32_t>& getIndices() const { return m_Indices; }

    VkBuffer& getVertexBuffer() { return m_VertexBuffer; }
    VkBuffer& getIndexBuffer() { return m_IndexBuffer; }

    friend class ModelImporter;
};


