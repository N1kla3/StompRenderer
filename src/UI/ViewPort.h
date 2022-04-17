#pragma once
#include "ImguiUnit.h"
#include "imgui.h"

namespace omp{
class ViewPort : public ImguiUnit
{
    ImVec2 m_Size;
    ImVec2 m_Offset;
    bool m_Resized = false;

public:
    virtual void renderUI() override;

    ImVec2 GetSize() const { return m_Size; };
    ImVec2 GetOffset() const { return m_Offset; }
    bool IsResized() const { return m_Resized; }
};
} // omp

