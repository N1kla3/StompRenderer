#include "Rendering/TextureSrc.h"
#include "Core/Profiling.h"

void omp::TextureSrc::serialize(JsonParser<>& parser)
{
    parser.writeValue("PathToTexture", m_Path);
}

void omp::TextureSrc::deserialize(JsonParser<>& parser)
{
    m_Path = parser.readValue<std::string>("PathToTexture").value_or(""); 
    m_IsLoaded = loadTextureFromFile();
}

omp::TextureSrc::~TextureSrc()
{
    if (m_IsLoaded)
    {
        stbi_image_free(m_Pixels);
    }
}

void omp::TextureSrc::setPath(const std::string& path)
{
    m_Path = path;
}

void omp::TextureSrc::tryLoad()
{
    if (m_Pixels)
    {
        stbi_image_free(m_Pixels);
    }
    m_IsLoaded = loadTextureFromFile();
}

bool omp::TextureSrc::loadTextureFromFile()
{
    OMP_STAT_SCOPE("LoadTexture");

    if (m_Path.empty())
    {
        VWARN(LogRendering, "Path to load texture is empty");
        return false;
    }

    int tex_channels;
    m_Pixels = stbi_load(m_Path.c_str(), &m_Width, &m_Height, &tex_channels, STBI_rgb_alpha);

    m_Size = m_Width * m_Height * 4;
    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

    if (!m_Pixels[0])
    {
        ERROR(LogRendering, "Failed to load texture image from path: {}", m_Path.c_str());
        return false;
    }
    return true;
}
