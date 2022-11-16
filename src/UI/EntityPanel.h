#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Model.h"
#include "MaterialPanel.h"

namespace omp
{
    class EntityPanel : public ImguiUnit
    {
        std::weak_ptr<Model> m_Model;
        std::weak_ptr<MaterialPanel> m_MaterialPanelRef;

    public:
        explicit EntityPanel(const std::shared_ptr<MaterialPanel>& materialPanel);
        virtual void renderUi(float deltaTime) override;
        void setModel(const std::shared_ptr<omp::Model>& model);
    };
}


