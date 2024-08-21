#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Scene.h"
#include "EntityPanel.h"
#include "MaterialPanel.h"

namespace omp
{
    class ScenePanel : public ImguiUnit
    {
    private:
        bool m_ClickedFromViewport = false;

        Scene* m_Scene = nullptr;
        std::shared_ptr<MaterialPanel> m_MaterialPanel;

    public:
        explicit ScenePanel(const std::shared_ptr<MaterialPanel>& inMatPanel);
        virtual void renderUi(float deltaTime) override;

        void setScene(Scene* inScene) { m_Scene = inScene; }
    };
}

