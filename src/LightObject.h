#pragma once

#include "Rendering/Model.h"
#include "Light.h"

template<typename T>
concept LightClassReq =
std::is_base_of_v<omp::GlobalLight, T> ||
std::is_base_of_v<omp::PointLight, T> ||
std::is_base_of_v<omp::SpotLight, T>;

namespace omp
{
    template<LightClassReq LightType>
    class LightObject
    {
    private:
        LightType m_Light;
        std::shared_ptr<Model> m_Model;

    public:
        using LightClass = LightType;

        LightObject();
        LightClass& getLight();

        inline void updateLightObject(){};
        void setModel(const std::shared_ptr<Model>& inModel);
    };
}

template<LightClassReq LightType>
omp::LightObject<LightType>::LightObject()
{
    m_Light = LightType{};
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
inline void omp::LightObject<omp::PointLight>::updateLightObject()
{
    auto& v = m_Model->getPosition();
    m_Light.position.x = v.x;
    m_Light.position.y = v.y;
    m_Light.position.z = v.z;
}

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

template<LightClassReq LightType>
void omp::LightObject<LightType>::setModel(const std::shared_ptr<Model>& inModel)
{
    m_Model = (inModel);
}
