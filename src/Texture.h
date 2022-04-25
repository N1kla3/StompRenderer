#pragma once
#include <string>
#include "stb_image.h"

namespace omp{
class Texture
{
    std::string m_ContentPath;
    stbi_uc* m_Pixels;
    int m_Size;
    uint32_t m_MipLevels;

public:
    Texture() = default;
    explicit Texture(const std::string& path);

    void LoadTexture(const std::string& path);
};
} // omp