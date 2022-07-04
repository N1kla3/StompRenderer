#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "imgui.h"

namespace omp{

class Camera;

class ViewPort : public ImguiUnit
{
    ImVec2 m_Size;
    ImVec2 m_Offset;
    bool m_Resized = false;

    std::shared_ptr<omp::Camera> m_Camera;

public:
    virtual void renderUI(float DeltaTime) override;

    ImVec2 GetSize() const { return m_Size; };
    ImVec2 GetOffset() const { return m_Offset; }
    bool IsResized() const { return m_Resized; }
    void SetCamera(const std::shared_ptr<omp::Camera>& camera) { m_Camera = camera; };
};
} // omp

