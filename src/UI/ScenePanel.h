#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Scene.h"
#include "EntityPanel.h"

namespace omp{
class ScenePanel : public ImguiUnit
{
    int16_t m_SelectedIndex;
    std::weak_ptr<Scene> m_Scene;
    std::shared_ptr<EntityPanel> m_EntityUI;

public:
    explicit ScenePanel(const std::shared_ptr<EntityPanel>& entityPanel);
    virtual void renderUI(float DeltaTime) override;
    void SetScene(const std::shared_ptr<Scene>& inScene) { m_Scene = inScene; }
};
}

