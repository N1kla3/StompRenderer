#include "CameraPanel.h"
#include "Camera.h"
#include "imgui.h"

void omp::CameraPanel::update(omp::Camera* camera)
{

    ImGui::Begin("Camera Panel");

    if (camera)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.5, 0.5, 0.5});

        float val = camera->getSpeed();
        bool res = ImGui::DragFloat("Speed", &val, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        if (res) camera->setSpeed(val);

        val = camera->getSens();
        res = ImGui::DragFloat("Sensitivity", &val, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        if (res) camera->setSens(val);

        glm::vec3 pos = camera->getPosition();
        res = ImGui::DragFloat3("Position", &pos[0], 0.1f, 0.0f, 0.0f, "%.2f", 0);
        if (res) camera->setPosition(pos);

        val = camera->getViewAngle();
        res = ImGui::DragFloat("View angle", &val, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        if (res) camera->setViewAngle(val);

        val = camera->getNearClipping();
        res = ImGui::DragFloat("Near clip distance", &val, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        if (res) camera->setNearClip(val);

        val = camera->getFarClipping();
        res = ImGui::DragFloat("Far clip distance", &val, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        if (res) camera->setFarClip(val);

        ImGui::PopStyleColor(1);
    }
    else
    {
        ImGui::BulletText("No camera");
    }

    ImGui::End();
}

