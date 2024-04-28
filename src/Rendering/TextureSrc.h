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
            m_IsLoaded = loadTextureFromFile();
        }
        ~TextureSrc();

        void setPath(const std::string& path);
        void tryLoad();
        int getSize() const { return m_Size; }
        int getWidth() const { return m_Width; }
        int getHeight() const { return m_Height; }
        uint32_t getMipLevels() const { return m_MipLevels; }
        bool isLoaded() const { return m_IsLoaded; }
        stbi_uc* getPixels() const { return m_Pixels; }

    private:
        std::string m_Path;
        stbi_uc* m_Pixels = nullptr;
        int m_Size;
        int m_Width, m_Height;
        uint32_t m_MipLevels;
        bool m_IsLoaded = false;

        virtual void serialize(JsonParser<>& parser) override;
        virtual void deserialize(JsonParser<>& parser) override;
    private:
        bool loadTextureFromFile();
    };
}
