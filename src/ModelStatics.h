#pragma once
#include <memory>
#include "Model.h"

namespace omp{

    struct ModelStatics
    {
        static std::shared_ptr<omp::Model> LoadModel(const std::string& name, const std::string& modelName);
    };
}
