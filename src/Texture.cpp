#include "Texture.h"
#include "stb_image.h"

omp::Texture::Texture(const std::string &path)
{
    LoadTexture(path);
}

void omp::Texture::LoadTexture(const std::string &path)
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    VkDeviceSize image_size = tex_width * tex_height * 4;

    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }
}
