#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/glm.hpp"
#include <functional>
#include <unordered_map>

namespace std
{
    template<>
    struct hash<glm::vec2 >
    {
        size_t operator()(glm::vec2 const& v) const
        {
            return ((hash<glm::vec2::value_type>()(v[0]) ^
                     (hash<glm::vec2::value_type>()(v[1]) << 1)) >> 1);
        }
    };

    template<>
    struct hash<glm::vec3 >
    {
        size_t operator()(glm::vec3 const& v) const
        {

            return ((hash<glm::vec3::value_type>()(v[0]) ^
                     (hash<glm::vec3::value_type>()(v[1]) << 1)) >> 1) ^
                   (hash<glm::vec3::value_type>()(v[2]) << 1);
        }
    };
}