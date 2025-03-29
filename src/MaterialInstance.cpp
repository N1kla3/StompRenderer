// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#include "MaterialInstance.h"
#include "Rendering/Material.h"

omp::MaterialInstance::MaterialInstance(const std::shared_ptr<omp::Material>& materialCreateFrom)
    : m_StaticMaterial(materialCreateFrom)
{
}
