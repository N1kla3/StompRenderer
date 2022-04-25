#include <cmath>
#include <stdexcept>
#include "Texture.h"
#include "stb_image.h"

omp::Texture::Texture(const std::string &path)
{
    LoadTexture(path);
}

void omp::Texture::LoadTexture(const std::string &path)
{
    m_ContentPath = path;
    int tex_width, tex_height, tex_channels;
    m_Pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    m_Size = tex_width * tex_height * 4;

    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

    if (!m_Pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }
}
