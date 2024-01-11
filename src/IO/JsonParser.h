#pragma once
#include <string>
#include <optional>

namespace omp
{
    template< typename ParserType >
    class JsonParser
    {
    private:
        ParserType m_Parser;

    public:
        JsonParser() = default;
        JsonParser(JsonParser&& other);
        JsonParser& operator=(JsonParser&& other);

        JsonParser(const JsonParser& other) = delete;
        JsonParser& operator=(const JsonParser& other) = delete;

    public:
        // Usage //
        // ===== //
        void PopulateFromFile(const std::string& filePath);
        void WriteToFile(const std::string& filePath);

        template< typename T >
        std::optional<T> ReadValue(const std::string& inKey) const;

        template< typename T >
        void WriteValue(const std::string& inKey, T&& value);

        template< typename T >
        bool Contains(const std::string& inKey) const;

    };
}
