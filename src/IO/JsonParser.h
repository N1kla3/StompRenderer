#pragma once
#include <string>
#include <optional>
#include <fstream>
#include "Logs.h"
#include "nlohmann/json.hpp"

namespace omp
{
    class nlohmannjson
    {
    private:
        nlohmann::json m_Data;
    public:
        using ImplementationType = nlohmann::json;
        nlohmannjson() = default;
        nlohmannjson(nlohmann::json&& value)
        {
            m_Data = value;
        }
        nlohmannjson(nlohmannjson&& other)
        {
            m_Data = std::move(other.m_Data);
        }
        nlohmannjson& operator=(nlohmannjson&& other)
        {
            m_Data = std::move(other.m_Data);
            return *this;
        }

        nlohmannjson(const nlohmannjson& other) = delete;
        nlohmannjson& operator=(const nlohmannjson& other) = delete;

        bool readJsonFromFile(const std::string& inFilePath)
        {
            std::ifstream f(inFilePath);
            if (f.is_open())
            {
                m_Data = nlohmann::json::parse(f);
                return true;
            }
            else
            {
                VWARN(LogIO, "Cant read json file {1}", inFilePath);
                return false;
            }
        }

        bool writeJsonToFile(const std::string& inFilePath)
        {
            std::ofstream f(inFilePath);
            if (f.is_open())
            {
                f << std::setw(4) << m_Data << std::endl;
                return true;
            }
            else
            {
                VWARN(LogIO, "Cant write json to file: {1}", inFilePath);
                return false;
            }
        }

        template< typename T >
        std::optional<T> read(const std::string& inKey) const
        {
            if (m_Data.contains(inKey))
            {
                T res = m_Data[inKey].template get<T>();
                return std::optional<T>(res);
            }
            return std::nullopt;
        }

        template< typename T >
        void write(const std::string& inKey, T&& inValue)
        {
            m_Data[inKey] = std::forward<T>(inValue);
        }

        bool contains(const std::string& inKey) const
        {
            return m_Data.contains(inKey);
        }

        ImplementationType getImplementation()
        {
            return m_Data;
        }
    };


    template<typename ParserType = nlohmannjson>
    class JsonParser
    {
    private:
        ParserType m_Parser;

    public:
        JsonParser() = default;
        JsonParser(ParserType&& parser)
        {
            m_Parser = std::move(parser);
        }
        JsonParser(JsonParser&& other)
        {
            m_Parser = std::move(other.m_Parser);
        }
        JsonParser& operator=(JsonParser&& other)
        {
            m_Parser = std::move(other.m_Parser);
            return *this;
        }

        JsonParser(const JsonParser& other) = delete;
        JsonParser& operator=(const JsonParser& other) = delete;


    public:
        // Usage //
        // ===== //
        bool populateFromFile(const std::string& filePath);
        bool writeToFile(const std::string& filePath);

        template<typename T>
        std::optional<T> readValue(const std::string& inKey) const;
        // Read json object
        JsonParser readObject(const std::string& inKey) const;

        template<typename T>
        void writeValue(const std::string& inKey, T&& value);
        // Add json object
        void writeObject(const std::string& inKey, JsonParser&& parser);

        bool contains(const std::string& inKey) const;

    };

    template< typename ParserType >
    bool JsonParser<ParserType>::populateFromFile(const std::string& filePath)
    {
        return m_Parser.readJsonFromFile(filePath);
    }

    template< typename ParserType >
    bool JsonParser<ParserType>::writeToFile(const std::string& filePath)
    {
        return m_Parser.writeJsonToFile(filePath);
    }

    template< typename ParserType >
    template< typename T >
    std::optional<T> JsonParser<ParserType>::readValue(const std::string& inKey) const
    {
        return m_Parser.template read<T>(inKey);
    }

    template< typename ParserType >
    JsonParser<ParserType> JsonParser<ParserType>::readObject(const std::string& inKey) const
    {
        return JsonParser(ParserType(m_Parser.template read<typename ParserType::ImplementationType>(inKey).value()));
    }

    template< typename ParserType >
    template< typename T >
    void JsonParser<ParserType>::writeValue(const std::string& inKey, T&& value)
    {
        m_Parser.template write<T>(inKey, std::forward<T>(value));
    }

    template< typename ParserType >
    void JsonParser<ParserType>::writeObject(const std::string& inKey, JsonParser&& parser)
    {
        m_Parser.template write<typename ParserType::ImplementationType>(inKey, parser.m_Parser.getImplementation());
    }

    template< typename ParserType >
    bool JsonParser<ParserType>::contains(const std::string& inKey) const
    {
        return m_Parser.contains(inKey);
    }
}
