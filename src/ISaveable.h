#pragma once


#include <string>

namespace omp
{
    class ISaveable
    {
    public:
        virtual void saveAssetToFile(const std::string& path) = 0;
    };
}