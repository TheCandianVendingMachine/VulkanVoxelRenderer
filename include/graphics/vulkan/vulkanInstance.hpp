// vulkanInstance.hpp
// Defines a wrapper around VkInstance
#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

class vulkanInstance
    {
        private:
            VkInstance m_instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

            bool m_validationLayersEnabled = false;
            bool m_cleanedUp = false;

        public:
            vulkanInstance() = default;
            vulkanInstance(VkApplicationInfo appInfo, const std::vector<const char*> &extensions);
            ~vulkanInstance();
            void create(VkApplicationInfo appInfo, const std::vector<const char*> &extensions);
            void cleanup();

            VkInstance getUnderlyingInstance() const;
            operator VkInstance() const;

            VkInstance &getUnderlyingInstance();
    };
