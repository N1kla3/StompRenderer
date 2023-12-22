#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/glm.hpp"

namespace std
{
    template<typename T, glm::qualifier Q>
    struct hash<glm::vec<2, T, Q> >
    {
        size_t operator()(glm::vec<2, T, Q> const& v) const
        {
            return ((hash<T>()(v[0]) ^
                     (hash<T>()(v[1]) << 1)) >> 1);
        }
    };

    template<typename T, glm::qualifier Q>
    struct hash<glm::vec<3, T, Q> >
    {
        size_t operator()(glm::vec<3, T, Q> const& v) const
        {

            return ((hash<T>()(v[0]) ^
                     (hash<T>()(v[1]) << 1)) >> 1) ^
                   (hash<T>()(v[2]) << 1);
        }
    };
}