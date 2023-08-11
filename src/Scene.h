#pragma once

#include <vector>
#include "Rendering/Model.h"
#include "Camera.h"
#include "Rendering/ModelInstance.h"
#include "SceneEntity.h"

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

        int32_t m_CurrentEntityId = -1;

        std::vector<std::shared_ptr<omp::SceneEntity>> m_Entities;
        std::shared_ptr<omp::Camera> m_CurrentCamera;

        std::vector<std::shared_ptr<omp::Camera>> m_Cameras;

    public:
        // Methods //
        // ======= //
        void addEntityToScene(const omp::SceneEntity& modelToAdd);
        void addEntityToScene(const std::shared_ptr<omp::SceneEntity>& modelToAdd);
        std::shared_ptr<omp::SceneEntity> getEntity(const std::string& entity);
        std::shared_ptr<omp::SceneEntity> getEntity(int32_t entity);

        // TODO map, no ref
        std::vector<std::shared_ptr<omp::SceneEntity>>& getEntities();

        std::shared_ptr<omp::Camera> getCurrentCamera() const { return m_CurrentCamera; }

        void setCurrentId(int32_t inId) { m_CurrentEntityId = inId; }
        int32_t getCurrentId() const { return m_CurrentEntityId; }

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
