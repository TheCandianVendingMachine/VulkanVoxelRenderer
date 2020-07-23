// vulkanSampler.hpp
// A wrapper around VkSampler
#pragma once
#include <vulkan/vulkan.h>

class vulkanSampler
    {
        private:
            VkSampler m_sampler = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = false;

        public:
            vulkanSampler() = default;
            vulkanSampler(VkDevice device, float maxLOD);
            ~vulkanSampler();
            void create(VkDevice device, float maxLOD);
            void cleanup();

            VkSampler getUnderlyingSampler() const;
            operator VkSampler() const;

            VkSampler &getUnderlyingSampler();

    };