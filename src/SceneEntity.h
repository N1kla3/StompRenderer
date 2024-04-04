#pragma once

#include <cstdint>
#include <string>
#include "UI/IDrawable.h"
#include "Rendering/ModelInstance.h"

namespace omp
{
    class SceneEntity : public IDrawable
    {
    private:
        int32_t m_Id;
    protected:
        std::string m_Name;
        // TODO temporarily
        std::shared_ptr<omp::ModelInstance> m_Model;

    public:
        SceneEntity();
        SceneEntity(const std::string& inName, const std::shared_ptr<omp::ModelInstance>& inModel);
        virtual ~SceneEntity() = default;
        int32_t getId() const { return m_Id; }
        std::string getName() const { return m_Name; }
        std::shared_ptr<omp::ModelInstance> getModel() const { return m_Model; }

        virtual void draw() override;

        virtual void onSceneSave(JsonParser<>& parser);
        virtual void onSceneLoad(JsonParser<>& parser);
    };
}
