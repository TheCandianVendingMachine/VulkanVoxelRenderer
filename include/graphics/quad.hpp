// quad.hpp
// A basic quad structure
#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct quad
    {
        glm::vec3 m_position;
        glm::vec2 m_size;
        char m_orientation = 0; // +-1 = xy, +-2 = xz, +-3 = yz
    };
