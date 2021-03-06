// vulkanAllocator.hpp
// A wrapper around AMD's VMA to ensure proper object creation and destruction
#pragma once
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>

class vulkanAllocator
    {
        private:
            VmaAllocator m_allocator = VK_NULL_HANDLE;
            
            bool m_cleanedUp = true;

        public:
            vulkanAllocator() = default;
            vulkanAllocator(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice);
            ~vulkanAllocator();
            void create(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice);
            void cleanup();

            bool isCreated() const;

            operator VmaAllocator() const;
            VmaAllocator getUnderlyingAllocator() const;

            VmaAllocator &getUnderlyingAllocator();
    };

namespace globals
    {
        extern vulkanAllocator *g_vulkanAllocator;
    }

