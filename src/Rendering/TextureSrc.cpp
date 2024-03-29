#include "Rendering/TextureSrc.h"

void omp::TextureSrc::serialize(JsonParser<>& parser)
{
    parser.writeValue("PathToTexture", m_Path);
}

void omp::TextureSrc::deserialize(JsonParser<>& parser)
{
    m_Path = parser.readValue<std::string>("PathToTexture").value_or(""); 
    loadTextureFromFile();
}

void omp::TextureSrc::loadTextureFromFile()
{
    if (m_Path.empty())
    {
        VWARN(LogRendering, "Path to load texture is empty");
        return;
    }

    int tex_channels;
    m_Pixels = stbi_load(m_Path.c_str(), &m_Width, &m_Height, &tex_channels, STBI_rgb_alpha);

    m_Size = m_Width * m_Height * 4;
    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

    if (!m_Pixels[0])
    {
        ERROR(LogRendering, "Failed to load texture image from path: {}", m_Path.c_str());
        throw std::runtime_error("Failed to load texture image!");
    }
}
