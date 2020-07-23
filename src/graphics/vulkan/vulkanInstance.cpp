#include "graphics/vulkan/vulkanInstance.hpp"
#include "graphics/vulkan/vulkanHelpers.hpp"
#include "graphics/vulkan/vulkanConsts.hpp"

vulkanInstance::vulkanInstance(VkApplicationInfo appInfo, const std::vector<const char*> &extensions)
    {
        create(appInfo, extensions);
    }

vulkanInstance::~vulkanInstance()
    {
        cleanup();
    }

void vulkanInstance::create(VkApplicationInfo appInfo, const std::vector<const char*> &extensions)
    {
        m_validationLayersEnabled = consts::c_enableValidationLayers;

        {
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            if (m_validationLayersEnabled)
                {
                    createInfo.enabledLayerCount = consts::c_validationLayersSize;
                    createInfo.ppEnabledLayerNames = consts::c_validationLayers;

                    helpers::populateDebugMessengerCreateInfo(debugCreateInfo);
                    debugCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
                
                }
        
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
                {
                    // <error>
                    return;
                }
        }

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> loadedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, loadedExtensions.data());

        for (uint32_t i = 0; i < extensions.size(); i++)
            {
                bool found = false;
                for (const auto &extension : loadedExtensions)
                    {
                        if (std::strncmp(extensions[i], extension.extensionName, sizeof(extension.extensionName) / sizeof(extension.extensionName[0])) == 0)
                            {
                                found = true;
                                break;
                            }
                    }

                if (!found)
                    {
                        // <error>
                        break;
                    }
            }

        if (m_validationLayersEnabled)
            {
                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                helpers::populateDebugMessengerCreateInfo(createInfo);

                auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
                if (func != nullptr)
                    {
                        if (func(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
                            {
                                // <error>
                                return;
                            }
                    }
                else
                    {
                        // <error>
                        return;
                    }
            }

        m_cleanedUp = false;
    }

void vulkanInstance::cleanup()
    {
        if (m_cleanedUp) { return; }
        if (m_validationLayersEnabled)
            {
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
                if (func != nullptr)
                    {
                        func(m_instance, m_debugMessenger, nullptr);
                    }
            }
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
        m_cleanedUp = true;
    }

VkInstance vulkanInstance::getUnderlyingInstance() const
    {
        return m_instance;
    }

vulkanInstance::operator VkInstance() const
    {
        return m_instance;
    }

VkInstance &vulkanInstance::getUnderlyingInstance()
    {
        return m_instance;
    }
