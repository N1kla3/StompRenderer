#pragma once
#include "Model.h"

namespace omp{
    class ModelImporter
    {
    public:
        [[nodiscard]] static bool loadModel(omp::Model* model, const std::string& path);
    };
}
