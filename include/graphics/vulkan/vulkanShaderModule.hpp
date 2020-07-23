// vulkanShaderModule.hpp
// A wrapper around VkShaderModule. Handles the initialisation of shaders within code
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class vulkanShaderModule
    {
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            VkShaderModule m_shaderModule = VK_NULL_HANDLE;

            bool m_cleanedUp = false;
        public:
            vulkanShaderModule() = default;
            vulkanShaderModule(VkDevice device, const std::vector<char> &code);
            ~vulkanShaderModule();
            void create(VkDevice device, const std::vector<char> &code);
            void cleanup();

            VkShaderModule getUnderlyingShaderModule() const;
            operator VkShaderModule() const;

            VkShaderModule &getUnderlyingShaderModule();

    };
