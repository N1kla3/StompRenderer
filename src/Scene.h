#pragma once

#include <vector>
#include "Rendering/Model.h"

namespace omp
{
    class Scene
    {
    public:
        Scene() = default;

    private:
        // State //
        // ===== //
        bool m_StateDirty = false;

        std::vector<std::shared_ptr<omp::Model>> m_Models;

    public:
        // Methods //
        // ======= //
        void addModelToScene(const omp::Model& modelToAdd);
        void addModelToScene(const std::shared_ptr<omp::Model>& modelToAdd);
        std::shared_ptr<omp::Model> getModel(const std::string& inName);

        // TODO map, no ref
        std::vector<std::shared_ptr<omp::Model>>& getModels();

        bool isDirty() const
        {
            return m_StateDirty;
        }

        void confirmRendering()
        {
            m_StateDirty = true;
        };
    };
} // omp
