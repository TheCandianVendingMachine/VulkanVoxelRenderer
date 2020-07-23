// vertex.hpp
// Defines a vertex of a mesh in 3d space
#pragma once
#include <array>
#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct vertex
    {
        glm::vec3 m_position;
        glm::vec3 m_colour;
        glm::vec2 m_textureCoord;

        static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};

                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
            {
                std::array<VkVertexInputAttributeDescription, 3> attributeDescription{};

                attributeDescription[0].binding = 0;
                attributeDescription[0].location = 0;
                attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescription[0].offset = offsetof(vertex, m_position);

                attributeDescription[1].binding = 0;
                attributeDescription[1].location = 1;
                attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescription[1].offset = offsetof(vertex, m_colour);
                
                attributeDescription[2].binding = 0;
                attributeDescription[2].location = 2;
                attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescription[2].offset = offsetof(vertex, m_textureCoord);

                return attributeDescription;
            }

        inline bool operator==(const vertex &rhs) const
            {
                return m_position == rhs.m_position && m_colour == rhs.m_colour && m_textureCoord == rhs.m_textureCoord;
            }
    };
