#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Scene.h"
#include "EntityPanel.h"

namespace omp
{
    class ScenePanel : public ImguiUnit
    {
        int16_t m_SelectedIndex;
        std::weak_ptr<Scene> m_Scene;
        std::shared_ptr<EntityPanel> m_EntityUi;

    public:
        explicit ScenePanel(const std::shared_ptr<EntityPanel>& entityPanel);
        virtual void renderUi(float deltaTime) override;

        void setScene(const std::shared_ptr<Scene>& inScene) { m_Scene = inScene; }
    };
}

