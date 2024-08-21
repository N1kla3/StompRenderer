#include "UIController.h"
#include "Scene.h"
#include "UI/CameraPanel.h"
#include "UI/EntityPanel.h"
#include "UI/ScenePanel.h"
#include "UI/MainLayer.h"

void omp::UIController::update(const omp::UIData& data, float deltaTime)
{
    return;
    // Should be first
    omp::MainLayer::update();
    omp::ScenePanel::update(data.scene);
    if (data.scene)
    {
        omp::CameraPanel::update(data.scene->getCurrentCamera());
        omp::EntityPanel::update(data.scene->getCurrentEntity());
    }
    else
    {
        omp::CameraPanel::update(nullptr);
        omp::EntityPanel::update(nullptr);
    }
}
