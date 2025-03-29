// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#pragma once

#include <memory>
#include "LightObject.h"
#include "ImguiUnit.h"

namespace omp
{

    struct GlobalLight;

    class GlobalLightPanel : public ImguiUnit
    {
    private:
        std::weak_ptr<omp::LightObject<omp::GlobalLight>> m_LightRef;

    public:
        GlobalLightPanel() = default;
        explicit GlobalLightPanel(const std::shared_ptr<omp::LightObject<omp::GlobalLight>>& inLight);
        void setLightRef(const std::shared_ptr<omp::LightObject<omp::GlobalLight>>& lightPtr);

        virtual void renderUi(float deltaTime);
    };
}
