#pragma once

#include "IO/SerializableObject.h"
#include "IO/stb_image.h"

namespace omp
{
    class TextureSrc : public SerializableObject
    {
    public:
        std::string m_Path;
        std::vector<stbi_uc*> m_Pixels{};
        int m_Size;
        int m_Width, m_Height;
        
        virtual void serialize(JsonParser<>& parser) override;
        virtual void deserialize(JsonParser<>& parser) override;
    };
}
