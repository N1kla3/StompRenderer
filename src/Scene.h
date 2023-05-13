#pragma once

#include <vector>
#include "Rendering/Model.h"
#include "Camera.h"

namespace omp
{
    class Scene
    {
    public:
        Scene();

    private:
        // State //
        // ===== //
        bool m_StateDirty = false;

        std::vector<std::shared_ptr<omp::Model>> m_Models;
        std::shared_ptr<omp::Camera> m_CurrentCamera;

        std::vector<std::shared_ptr<omp::Camera>> m_Cameras;

    public:
        // Methods //
        // ======= //
        void addModelToScene(const omp::Model& modelToAdd);
        void addModelToScene(const std::shared_ptr<omp::Model>& modelToAdd);
        std::shared_ptr<omp::Model> getModel(const std::string& inName);

        // TODO map, no ref
        std::vector<std::shared_ptr<omp::Model>>& getModels();

        std::shared_ptr<omp::Camera> getCurrentCamera() const { return m_CurrentCamera; }

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
