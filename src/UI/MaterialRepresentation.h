#pragma once

#include "AssetRepresentation.h"
#include <memory>
#include "Rendering/Texture.h"


namespace omp
{
    class MaterialRepresentation : public AssetRepresentation
    {
    private:
        std::shared_ptr<Texture> m_Texture;

    public:
        MaterialRepresentation(const std::shared_ptr<Texture>& texture);
        virtual ~MaterialRepresentation() = default;
        void setDependency(const std::shared_ptr<Texture>& texture);

        virtual void renderUIonPanel() override;
        virtual void renderUIonContentBrowser() override;
    };
}