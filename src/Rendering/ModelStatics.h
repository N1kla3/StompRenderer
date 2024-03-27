#pragma once
#include "Model.h"

namespace omp{
    class ModelImporter
    {
    public:
        static void loadModel(omp::Model* model, const std::string& path);
    };
}
