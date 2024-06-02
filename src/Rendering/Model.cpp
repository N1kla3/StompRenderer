#include "Model.h"
#include "Rendering/ModelStatics.h"

omp::Model::Model()
        : m_Name("NONE")
{

}

omp::Model::Model(const std::string& path)
        : m_Name("NONE")
{
    omp::ModelImporter::loadModel(this, path);
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

void omp::Model::loadVertexToMemory()
{
    VkDeviceSize buffer_size = sizeof(getVertices()[0]) * getVertices().size();

    // Vertex buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    if (m_Context.expired())
    {
        return;
    }

    std::shared_ptr<omp::VulkanContext> context = m_Context.lock();

    context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_memory);

    void* data;
    vkMapMemory(context->logical_device, staging_memory, 0, buffer_size, 0, &data);
    memcpy(data, getVertices().data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(context->logical_device, staging_memory);

    context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer, m_VertexMemory);

    context->copyBuffer(staging_buffer, m_VertexBuffer, buffer_size);

    vkDestroyBuffer(context->logical_device, staging_buffer, nullptr);
    vkFreeMemory(context->logical_device, staging_memory, nullptr);
}

void omp::Model::loadIndexToMemory()
{
    // Index buffer
    VkDeviceSize buffer_size = sizeof(getIndices()[0]) * getIndices().size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    std::shared_ptr<omp::VulkanContext> context = m_Context.lock();

    context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_memory);

    void* data;
    vkMapMemory(context->logical_device, staging_memory, 0, buffer_size, 0, &data);
    memcpy(data, getIndices().data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(context->logical_device, staging_memory);

    context->createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffer, m_IndexMemory);

    context->copyBuffer(staging_buffer, m_IndexBuffer, buffer_size);

    vkDestroyBuffer(context->logical_device, staging_buffer, nullptr);
    vkFreeMemory(context->logical_device, staging_memory, nullptr);
}

void omp::Model::tryClear()
{
    if (!m_Context.expired())
    {
        auto context = m_Context.lock();
        vkDestroyBuffer(context->logical_device, m_IndexBuffer, nullptr);
        vkDestroyBuffer(context->logical_device, m_VertexBuffer, nullptr);
        vkFreeMemory(context->logical_device, m_IndexMemory, nullptr);
        vkFreeMemory(context->logical_device, m_VertexMemory, nullptr);
    }
    m_Context.reset();
}

omp::Model::~Model()
{
    tryClear();
}

void omp::Model::loadToMemory(const std::shared_ptr<omp::VulkanContext>& context, bool forceUpdate)
{
    if (!m_Context.expired() && !forceUpdate)
    {
        return;
    }

    tryClear();

    m_Context = context;

    loadVertexToMemory();
    loadIndexToMemory();
}

