#pragma once
#include "Core/ICommand.h"
#include "ViewPort.h"

namespace omp
{
    struct UIData
    {
        class Scene* scene = nullptr;
        class Renderer* renderer = nullptr;
    };

    class UIController
    {
    public:
        UIController() = default;
        void update(const UIData& data, float deltaTime);

    private:
        CommandStack m_CommandStack{};
        omp::ViewPort m_Viewport{};
    };
}
