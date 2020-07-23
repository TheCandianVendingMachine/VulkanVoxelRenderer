// queueFamilyIndices.hpp
// A struct that contains the types of queues a physical device supports
#pragma once
#include <optional>

struct vulkanQueueFamilyIndices
    {
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        bool isComplete()
            {
                return m_graphicsFamily.has_value() && m_presentFamily.has_value();
            }
    };

