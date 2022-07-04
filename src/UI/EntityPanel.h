#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Model.h"
#include "MaterialPanel.h"

namespace omp{
class EntityPanel : public ImguiUnit
{
    std::weak_ptr<Model> m_Model;
    std::weak_ptr<MaterialPanel> m_MaterialPanelRef;

public:
    explicit EntityPanel(const std::shared_ptr<MaterialPanel>& materialPanel);
    virtual void renderUI(float DeltaTime) override;
    void SetModel(const std::shared_ptr<omp::Model>& model);
};
}


