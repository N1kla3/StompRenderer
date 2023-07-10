#pragma once
#include <memory>
#include "Model.h"
#include "ModelInstance.h"

namespace omp{

    class ModelManager
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<omp::Model>> m_Models;
    public:
        void loadModel(const std::string& inPath);
        std::shared_ptr<omp::Model> getModel(const std::string& inPath) const;
        std::shared_ptr<omp::ModelInstance> createInstanceFrom(const std::string& inPath);
    };
}
