#pragma once

namespace omp
{
    class AssetRepresentation
    {
    public:
        virtual void renderUIonPanel() = 0;
        virtual void renderUIonContentBrowser() = 0;
    };
}