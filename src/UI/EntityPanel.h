#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Rendering/Model.h"
#include "MaterialPanel.h"
#include "Rendering/ModelInstance.h"

namespace omp
{
    class EntityPanel : public ImguiUnit
    {
        std::weak_ptr<ModelInstance> m_Model;
        std::weak_ptr<MaterialPanel> m_MaterialPanelRef;

    public:
        explicit EntityPanel(const std::shared_ptr<MaterialPanel>& materialPanel);
        virtual void renderUi(float deltaTime) override;
        void setModel(const std::shared_ptr<omp::ModelInstance>& model);
    };
}


