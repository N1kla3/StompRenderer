#include "UIController.h"
#include "Scene.h"
#include "UI/CameraPanel.h"
#include "UI/EntityPanel.h"

void omp::UIController::update(const omp::UIData& data, float deltaTime)
{
    return;
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
