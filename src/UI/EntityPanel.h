#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Model.h"

namespace omp{
class EntityPanel : public ImguiUnit
{
    std::weak_ptr<omp::Model> m_Model;

public:
    virtual void renderUI() override;
    void SetModel(const std::shared_ptr<omp::Model>& model);
};
}


