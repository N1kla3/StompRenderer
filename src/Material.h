#pragma once
#include <vector>
#include <memory>
#include "Texture.h"

struct TextureData
{
    std::shared_ptr<omp::Texture> Texture;
};

namespace omp{
class Material
{
    std::vector<TextureData> m_Textures;
};
} // omp
