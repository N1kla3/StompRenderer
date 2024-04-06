#pragma once

#include <memory>
#include <vector>
#include "IO/SerializableObject.h"
#include "Camera.h"
#include "SceneEntity.h"

namespace omp
{
    class Scene : public omp::SerializableObject
    {
    public:
        Scene();

    private:
        // State //
        // ===== //
        std::vector<std::unique_ptr<omp::SceneEntity>> m_Entities;
        std::vector<std::unique_ptr<omp::SceneEntity>> m_Cameras;

        omp::Camera* m_CurrentCamera;

        bool m_StateDirty = false;
        int32_t m_CurrentEntityId = -1;

    public:
        // Methods //
        // ======= //
        void addEntityToScene(const omp::SceneEntity& modelToAdd);
        void addEntityToScene(std::unique_ptr<omp::SceneEntity>&& modelToAdd);
        omp::SceneEntity* getEntity(const std::string& entity) const;
        omp::SceneEntity* getEntity(int32_t entity) const;
        omp::SceneEntity* getCurrentEntity() const;

        virtual void serialize(JsonParser<>& parser) override;
        virtual void deserialize(JsonParser<>& parser) override;

        // TODO map, no ref
        std::vector<std::unique_ptr<omp::SceneEntity>>& getEntities();

        omp::Camera* getCurrentCamera() const { return m_CurrentCamera; }

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
