#include "Model.h"
#include "Rendering/ModelStatics.h"

omp::Model::Model()
        : m_Name("NONE")
{

}

void omp::Model::serialize(JsonParser<>& parser)
{
    parser.writeValue("ContentPath", m_Path);
}

void omp::Model::deserialize(JsonParser<>& parser)
{
    m_Path = parser.readValue<std::string>("ContentPath").value_or("");

    if (!m_Path.empty())
    {
        omp::ModelImporter::loadModel(this, m_Path);
    }
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

void omp::Model::loadVertexToMemory(const std::shared_ptr<omp::VulkanContext>& inContext)
{
    VkDeviceSize buffer_size = sizeof(getVertices()[0]) * getVertices().size();

    m_Context = inContext;

    // Vertex buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    m_Context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_memory);

    void* data;
    vkMapMemory(m_Context->logical_device, staging_memory, 0, buffer_size, 0, &data);
    memcpy(data, getVertices().data(), (size_t) buffer_size);
    vkUnmapMemory(m_Context->logical_device, staging_memory);

    m_Context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer, m_VertexMemory);

    m_Context->copyBuffer(staging_buffer, m_VertexBuffer, buffer_size);

    vkDestroyBuffer(m_Context->logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_Context->logical_device, staging_memory, nullptr);
}

void omp::Model::loadIndexToMemory(const std::shared_ptr<omp::VulkanContext>& inContext)
{
    // Index buffer
    VkDeviceSize buffer_size = sizeof(getIndices()[0]) * getIndices().size();

    m_Context = inContext;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    m_Context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_memory);

    void* data;
    vkMapMemory(m_Context->logical_device, staging_memory, 0, buffer_size, 0, &data);
    memcpy(data, getIndices().data(), (size_t) buffer_size);
    vkUnmapMemory(m_Context->logical_device, staging_memory);

    m_Context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffer, m_IndexMemory);

    m_Context->copyBuffer(staging_buffer, m_IndexBuffer, buffer_size);

    vkDestroyBuffer(m_Context->logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_Context->logical_device, staging_memory, nullptr);
}

omp::Model::~Model()
{
    if (m_Context)
    {
        vkDestroyBuffer(m_Context->logical_device, m_IndexBuffer, nullptr);
        vkDestroyBuffer(m_Context->logical_device, m_VertexBuffer, nullptr);
        vkFreeMemory(m_Context->logical_device, m_IndexMemory, nullptr);
        vkFreeMemory(m_Context->logical_device, m_VertexMemory, nullptr);
    }
}
