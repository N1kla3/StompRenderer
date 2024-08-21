#include "UIController.h"
#include "Scene.h"
#include "UI/CameraPanel.h"

void omp::UIController::update(const omp::UIData& data, float deltaTime)
{
    return;
    if (data.scene)
    {
        omp::CameraPanel::update(data.scene->getCurrentCamera());
    }
    else
    {
        omp::CameraPanel::update(nullptr);
    }
}
