// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#pragma once
#include "Model.h"

namespace omp
{
    class ModelImporter
    {
    public:
        [[nodiscard]] static bool loadModel(omp::Model* model, const std::string& path);
    };
}
