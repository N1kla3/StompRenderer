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