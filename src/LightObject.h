#pragma once

#include "Rendering/Model.h"
#include "Light.h"
#include "Rendering/ModelInstance.h"
#include "imgui.h"
#include "SceneEntity.h"

template<typename T>
concept LightClassReq =
std::is_base_of_v<omp::GlobalLight, T> ||
std::is_base_of_v<omp::PointLight, T> ||
std::is_base_of_v<omp::SpotLight, T>;

namespace omp
{
    template<LightClassReq LightType>
    class LightObject : public omp::SceneEntity
    {
    private:
        LightType m_Light;

    public:
        using LightClass = LightType;

        LightObject();
        LightObject(const std::string& inName);
        LightObject(const std::string& inName, const std::shared_ptr<omp::ModelInstance>& inModel);
        virtual ~LightObject() = default;
        LightClass& getLight();

        inline void updateLightObject(){};
        inline virtual void draw() override { SceneEntity::draw(); };
        inline virtual void onSceneSave(JsonParser<>& a, omp::Scene* b) override { SceneEntity::onSceneSave(a, b); };
        inline virtual void onSceneLoad(JsonParser<>& a, omp::Scene* b) override { SceneEntity::onSceneLoad(a, b); };
        inline virtual std::string getClassName() const override;
        void setModel(const std::shared_ptr<ModelInstance>& inModel);
    };
}

template<LightClassReq LightType>
omp::LightObject<LightType>::LightObject()
        : omp::SceneEntity()
{
}

template<LightClassReq LightType>
omp::LightObject<LightType>::LightObject(const std::string& inName)
    : omp::SceneEntity(inName, nullptr)
{
}

template<LightClassReq LightType>
omp::LightObject<LightType>::LightObject(const std::string& inName, const std::shared_ptr<omp::ModelInstance>& inModel)
        : omp::SceneEntity(inName, inModel)
{
}

template<LightClassReq LightType>
omp::LightObject<LightType>::LightClass& omp::LightObject<LightType>::getLight()
{
    return m_Light;
}

template<>
inline void omp::LightObject<omp::GlobalLight>::updateLightObject()
{
    auto& v = m_Model->getPosition();
    m_Light.position_or_direction.x = v.x;
    m_Light.position_or_direction.y = v.y;
    m_Light.position_or_direction.z = v.z;
}

template<>
inline void omp::LightObject<omp::GlobalLight>::draw()
{
    SceneEntity::draw();

    ImGui::Text("Light properties");
    ImGui::DragFloat4("Ambient", &m_Light.ambient.x, 0.1f);
    ImGui::DragFloat4("Diffuse", &m_Light.diffuse.x, 0.1f);
    ImGui::DragFloat4("Specular", &m_Light.specular.x, 0.1f);
}

template<>
inline void omp::LightObject<omp::GlobalLight>::onSceneSave(JsonParser<>& parser, omp::Scene* scene)
{
}

template<>
inline void omp::LightObject<omp::GlobalLight>::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
}

template<>
inline std::string omp::LightObject<omp::GlobalLight>::getClassName() const { return "GlobalLight"; }

template<>
inline void omp::LightObject<omp::PointLight>::updateLightObject()
{
    auto& v = m_Model->getPosition();
    m_Light.position.x = v.x;
    m_Light.position.y = v.y;
    m_Light.position.z = v.z;
}

template<>
inline void omp::LightObject<omp::PointLight>::draw()
{
    SceneEntity::draw();

    ImGui::Text("Light Properties");

    ImGui::DragFloat4("Ambient", &m_Light.ambient.x, 0.1f);
    ImGui::DragFloat4("Diffuse", &m_Light.diffuse.x, 0.1f);
    ImGui::DragFloat4("Specular", &m_Light.specular.x, 0.1f);

    ImGui::DragFloat("Constant", &m_Light.constant, 0.1f);
    ImGui::DragFloat("Linear", &m_Light.linear, 0.1f);
    ImGui::DragFloat("Quadratic", &m_Light.quadratic, 0.1f);
}

template<>
inline void omp::LightObject<omp::PointLight>::onSceneSave(JsonParser<>& parser, omp::Scene* scene)
{
}

template<>
inline void omp::LightObject<omp::PointLight>::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
}

template<>
inline std::string omp::LightObject<omp::PointLight>::getClassName() const { return "PointLight"; }

template<>
inline void omp::LightObject<omp::SpotLight>::updateLightObject()
{
    auto& pos = m_Model->getPosition();
    m_Light.position.x = pos.x;
    m_Light.position.y = pos.y;
    m_Light.position.z = pos.z;

    auto& rot = m_Model->getRotation();
    m_Light.direction.x = rot.x;
    m_Light.direction.y = rot.y;
    m_Light.direction.z = rot.z;
}

template<>
inline void omp::LightObject<omp::SpotLight>::draw()
{
    SceneEntity::draw();

    ImGui::Text("Light properties");
    ImGui::DragFloat4("Ambient", &m_Light.ambient.x, 0.1f);
    ImGui::DragFloat4("Diffuse", &m_Light.diffuse.x, 0.1f);
    ImGui::DragFloat4("Specular", &m_Light.specular.x, 0.1f);

    ImGui::DragFloat("Cut off", &m_Light.cut_off, 0.1f);
    ImGui::DragFloat("Outer cut off", &m_Light.outer_cutoff, 0.1f);
    ImGui::DragFloat("Constant", &m_Light.constant, 0.1f);
    ImGui::DragFloat("Linear", &m_Light.linear, 0.1f);
    ImGui::DragFloat("Quadratic", &m_Light.quadratic, 0.1f);
}

template<>
inline void omp::LightObject<omp::SpotLight>::onSceneSave(JsonParser<>& parser, omp::Scene* scene)
{
}

template<>
inline void omp::LightObject<omp::SpotLight>::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
}

template<>
inline std::string omp::LightObject<omp::SpotLight>::getClassName() const { return "SpotLight"; }

template<LightClassReq LightType>
void omp::LightObject<LightType>::setModel(const std::shared_ptr<ModelInstance>& inModel)
{
    m_Model = (inModel);
}
