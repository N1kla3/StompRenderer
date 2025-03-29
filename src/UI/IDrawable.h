// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#pragma once

namespace omp
{
    class IDrawable
    {
    public:
        virtual void draw() = 0;
        virtual ~IDrawable() = default;
    };
}
