// vulkanSampler.hpp
// A wrapper around VkSampler
#pragma once
#include <vulkan/vulkan.h>

class vulkanSampler
    {
        private:
            VkSampler m_sampler = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanSampler() = default;
            vulkanSampler(VkDevice device, float maxLOD, VkSamplerAddressMode samplerMode);
            ~vulkanSampler();
            void create(VkDevice device, float maxLOD, VkSamplerAddressMode samplerMode);
            void cleanup();

            bool isCreated() const;

            VkSampler getUnderlyingSampler() const;
            operator VkSampler() const;

            VkSampler &getUnderlyingSampler();

    };