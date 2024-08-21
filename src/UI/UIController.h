#pragma once
#include "Core/ICommand.h"

namespace omp
{
    struct UIData
    {
        class Scene* scene;
    };

    class UIController
    {
    public:
        UIController() = default;
        void update(const UIData& data, float deltaTime);

    private:
        CommandStack m_CommandStack{};
    };
}
