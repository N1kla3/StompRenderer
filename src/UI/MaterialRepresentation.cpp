// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#include "MaterialRepresentation.h"

omp::MaterialRepresentation::MaterialRepresentation(const std::shared_ptr<Texture>& texture)
    : m_Texture(texture)
{

}

void omp::MaterialRepresentation::setDependency(const std::shared_ptr<Texture>& texture)
{
    m_Texture = texture;
}

void omp::MaterialRepresentation::renderUIonPanel()
{

}

void omp::MaterialRepresentation::renderUIonContentBrowser()
{

}
