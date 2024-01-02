#pragma once

#include <string>

namespace omp
{
    class ISaveable
    {
    public:
        virtual void saveAssetToFile(const std::string& path, const std::string& inClassName) = 0;
        virtual ~ISaveable() = default;
    };
}