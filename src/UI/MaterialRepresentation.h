#pragma once
#include "AssetRepresentation.h"
#include <memory>
#include "Texture.h"


namespace omp
{
    class MaterialRepresentation : public AssetRepresentation
    {
    private:
        std::shared_ptr<Texture> m_Texture;

    public:
        MaterialRepresentation(const std::shared_ptr<Texture>& texture);
        void SetDependency(const std::shared_ptr<Texture>& texture);

        virtual void renderUIonPanel() override;
        virtual void renderUIonContentBrowser() override;
    };
}