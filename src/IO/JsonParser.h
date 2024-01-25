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
        nlohmannjson() = default;
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
                VWARN(IO, "Cant read json file {1}", inFilePath);
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
                VWARN(IO, "Cant write json to file: {1}", inFilePath);
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

    };


    template<typename ParserType = nlohmannjson>
    class JsonParser
    {
    private:
        ParserType m_Parser;

    public:
        JsonParser() = default;
        JsonParser(JsonParser&& other)
        {
            m_Parser = std::move(m_Parser);
        }
        JsonParser& operator=(JsonParser&& other)
        {
            m_Parser = std::move(m_Parser);
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

        template<typename T>
        void writeValue(const std::string& inKey, T&& value);

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
    template< typename T >
    void JsonParser<ParserType>::writeValue(const std::string& inKey, T&& value)
    {
        m_Parser.template write<T>(inKey, std::forward<T>(value));
    }

    template< typename ParserType >
    bool JsonParser<ParserType>::contains(const std::string& inKey) const
    {
        return m_Parser.contains(inKey);
    }
}
