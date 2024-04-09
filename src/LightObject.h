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
    SceneEntity::onSceneSave(parser, scene);

    parser.writeValue("posdir_x", m_Light.position_or_direction.x);
    parser.writeValue("posdir_y", m_Light.position_or_direction.y);
    parser.writeValue("posdir_z", m_Light.position_or_direction.z);
    parser.writeValue("posdir_w", m_Light.position_or_direction.w);

    parser.writeValue("ambient_x", m_Light.ambient.x);
    parser.writeValue("ambient_y", m_Light.ambient.y);
    parser.writeValue("ambient_z", m_Light.ambient.z);
    parser.writeValue("ambient_w", m_Light.ambient.w);

    parser.writeValue("diffuse_x", m_Light.diffuse.x);
    parser.writeValue("diffuse_y", m_Light.diffuse.y);
    parser.writeValue("diffuse_z", m_Light.diffuse.z);
    parser.writeValue("diffuse_w", m_Light.diffuse.w);

    parser.writeValue("specular_x", m_Light.specular.x);
    parser.writeValue("specular_y", m_Light.specular.y);
    parser.writeValue("specular_z", m_Light.specular.z);
    parser.writeValue("specular_w", m_Light.specular.w);
}

template<>
inline void omp::LightObject<omp::GlobalLight>::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
    SceneEntity::onSceneLoad(parser, scene);

    m_Light.position_or_direction.x = parser.readValue<float>("posdir_x").value();
    m_Light.position_or_direction.y = parser.readValue<float>("posdir_y").value();
    m_Light.position_or_direction.z = parser.readValue<float>("posdir_z").value();
    m_Light.position_or_direction.w = parser.readValue<float>("posdir_w").value();

    m_Light.ambient.x = parser.readValue<float>("ambient_x").value();
    m_Light.ambient.y = parser.readValue<float>("ambient_y").value();
    m_Light.ambient.z = parser.readValue<float>("ambient_z").value();
    m_Light.ambient.w = parser.readValue<float>("ambient_w").value();

    m_Light.diffuse.x = parser.readValue<float>("diffuse_x").value();
    m_Light.diffuse.y = parser.readValue<float>("diffuse_y").value();
    m_Light.diffuse.z = parser.readValue<float>("diffuse_z").value();
    m_Light.diffuse.w = parser.readValue<float>("diffuse_w").value();

    m_Light.specular.x = parser.readValue<float>("specular_x").value();
    m_Light.specular.y = parser.readValue<float>("specular_y").value();
    m_Light.specular.z = parser.readValue<float>("specular_z").value();
    m_Light.specular.w = parser.readValue<float>("specular_w").value();
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
    SceneEntity::onSceneSave(parser, scene);

    parser.writeValue("lpos_x", m_Light.position.x);
    parser.writeValue("lpos_y", m_Light.position.y);
    parser.writeValue("lpos_z", m_Light.position.z);
    parser.writeValue("lpos_w", m_Light.position.w);

    parser.writeValue("ambient_x", m_Light.ambient.x);
    parser.writeValue("ambient_y", m_Light.ambient.y);
    parser.writeValue("ambient_z", m_Light.ambient.z);
    parser.writeValue("ambient_w", m_Light.ambient.w);

    parser.writeValue("diffuse_x", m_Light.diffuse.x);
    parser.writeValue("diffuse_y", m_Light.diffuse.y);
    parser.writeValue("diffuse_z", m_Light.diffuse.z);
    parser.writeValue("diffuse_w", m_Light.diffuse.w);

    parser.writeValue("specular_x", m_Light.specular.x);
    parser.writeValue("specular_y", m_Light.specular.y);
    parser.writeValue("specular_z", m_Light.specular.z);
    parser.writeValue("specular_w", m_Light.specular.w);

    parser.writeValue("constant", m_Light.constant);
    parser.writeValue("linear", m_Light.linear);
    parser.writeValue("quadratic", m_Light.quadratic);
}

template<>
inline void omp::LightObject<omp::PointLight>::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
    SceneEntity::onSceneLoad(parser, scene);

    m_Light.position.x = parser.readValue<float>("lpos_x").value();
    m_Light.position.y = parser.readValue<float>("lpos_y").value();
    m_Light.position.z = parser.readValue<float>("lpos_z").value();
    m_Light.position.w = parser.readValue<float>("lpos_w").value();

    m_Light.ambient.x = parser.readValue<float>("ambient_x").value();
    m_Light.ambient.y = parser.readValue<float>("ambient_y").value();
    m_Light.ambient.z = parser.readValue<float>("ambient_z").value();
    m_Light.ambient.w = parser.readValue<float>("ambient_w").value();

    m_Light.diffuse.x = parser.readValue<float>("diffuse_x").value();
    m_Light.diffuse.y = parser.readValue<float>("diffuse_y").value();
    m_Light.diffuse.z = parser.readValue<float>("diffuse_z").value();
    m_Light.diffuse.w = parser.readValue<float>("diffuse_w").value();

    m_Light.specular.x = parser.readValue<float>("specular_x").value();
    m_Light.specular.y = parser.readValue<float>("specular_y").value();
    m_Light.specular.z = parser.readValue<float>("specular_z").value();
    m_Light.specular.w = parser.readValue<float>("specular_w").value();

    m_Light.constant = parser.readValue<float>("constant").value();
    m_Light.linear = parser.readValue<float>("linear").value();
    m_Light.quadratic = parser.readValue<float>("quadratic").value();
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
    SceneEntity::onSceneSave(parser, scene);

    parser.writeValue("lpos_x", m_Light.position.x);
    parser.writeValue("lpos_y", m_Light.position.y);
    parser.writeValue("lpos_z", m_Light.position.z);
    parser.writeValue("lpos_w", m_Light.position.w);

    parser.writeValue("ambient_x", m_Light.ambient.x);
    parser.writeValue("ambient_y", m_Light.ambient.y);
    parser.writeValue("ambient_z", m_Light.ambient.z);
    parser.writeValue("ambient_w", m_Light.ambient.w);

    parser.writeValue("diffuse_x", m_Light.diffuse.x);
    parser.writeValue("diffuse_y", m_Light.diffuse.y);
    parser.writeValue("diffuse_z", m_Light.diffuse.z);
    parser.writeValue("diffuse_w", m_Light.diffuse.w);

    parser.writeValue("specular_x", m_Light.specular.x);
    parser.writeValue("specular_y", m_Light.specular.y);
    parser.writeValue("specular_z", m_Light.specular.z);
    parser.writeValue("specular_w", m_Light.specular.w);

    parser.writeValue("direction_x", m_Light.direction.x);
    parser.writeValue("direction_y", m_Light.direction.y);
    parser.writeValue("direction_z", m_Light.direction.z);
    parser.writeValue("direction_w", m_Light.direction.w);

    parser.writeValue("cut_off", m_Light.cut_off);
    parser.writeValue("outer_cutoff", m_Light.outer_cutoff);

    parser.writeValue("constant", m_Light.constant);
    parser.writeValue("linear", m_Light.linear);
    parser.writeValue("quadratic", m_Light.quadratic);
}

template<>
inline void omp::LightObject<omp::SpotLight>::onSceneLoad(JsonParser<>& parser, omp::Scene* scene)
{
    SceneEntity::onSceneLoad(parser, scene);

    m_Light.position.x = parser.readValue<float>("lpos_x").value();
    m_Light.position.y = parser.readValue<float>("lpos_y").value();
    m_Light.position.z = parser.readValue<float>("lpos_z").value();
    m_Light.position.w = parser.readValue<float>("lpos_w").value();

    m_Light.ambient.x = parser.readValue<float>("ambient_x").value();
    m_Light.ambient.y = parser.readValue<float>("ambient_y").value();
    m_Light.ambient.z = parser.readValue<float>("ambient_z").value();
    m_Light.ambient.w = parser.readValue<float>("ambient_w").value();

    m_Light.diffuse.x = parser.readValue<float>("diffuse_x").value();
    m_Light.diffuse.y = parser.readValue<float>("diffuse_y").value();
    m_Light.diffuse.z = parser.readValue<float>("diffuse_z").value();
    m_Light.diffuse.w = parser.readValue<float>("diffuse_w").value();

    m_Light.specular.x = parser.readValue<float>("specular_x").value();
    m_Light.specular.y = parser.readValue<float>("specular_y").value();
    m_Light.specular.z = parser.readValue<float>("specular_z").value();
    m_Light.specular.w = parser.readValue<float>("specular_w").value();

    m_Light.direction.x = parser.readValue<float>("direction_x").value();
    m_Light.direction.y = parser.readValue<float>("direction_y").value();
    m_Light.direction.z = parser.readValue<float>("direction_z").value();
    m_Light.direction.w = parser.readValue<float>("direction_w").value();

    m_Light.cut_off = parser.readValue<float>("cut_off").value();
    m_Light.outer_cutoff = parser.readValue<float>("outer_cutoff").value();

    m_Light.constant = parser.readValue<float>("constant").value();
    m_Light.linear = parser.readValue<float>("linear").value();
    m_Light.quadratic = parser.readValue<float>("quadratic").value();
}

template<>
inline std::string omp::LightObject<omp::SpotLight>::getClassName() const { return "SpotLight"; }

template<LightClassReq LightType>
void omp::LightObject<LightType>::setModel(const std::shared_ptr<ModelInstance>& inModel)
{
    m_Model = (inModel);
}
