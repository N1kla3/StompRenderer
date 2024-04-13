#pragma once

#include "IO/SerializableObject.h"
#include "IO/stb_image.h"

namespace omp
{
    class TextureSrc : public SerializableObject
    {
    public:
        TextureSrc() = default;
        TextureSrc(const std::string& path)
            : m_Path(path)
    {
        loadTextureFromFile();
    }

    private:
        std::string m_Path;
        stbi_uc* m_Pixels{};
        int m_Size;
        int m_Width, m_Height;
        uint32_t m_MipLevels;

        virtual void serialize(JsonParser<>& parser) override;
        virtual void deserialize(JsonParser<>& parser) override;
    private:
        void loadTextureFromFile();
    };
}
