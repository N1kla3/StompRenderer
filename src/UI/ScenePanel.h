#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Scene.h"
#include "EntityPanel.h"

namespace omp
{
    class ScenePanel : public ImguiUnit
    {
    private:
        bool m_ClickedFromViewport = false;

        std::weak_ptr<Scene> m_Scene;
        std::shared_ptr<EntityPanel> m_EntityUi;
        std::shared_ptr<MaterialPanel> m_MaterialPanel;

    public:
        explicit ScenePanel(const std::shared_ptr<EntityPanel>& entityPanel, const std::shared_ptr<MaterialPanel>& inMatPanel);
        virtual void renderUi(float deltaTime) override;

        void setScene(const std::shared_ptr<Scene>& inScene) { m_Scene = inScene; }
    };
}

