#include "graphics/vulkan/vulkanShaderModule.hpp"

vulkanShaderModule::vulkanShaderModule(VkDevice device, const std::vector<char> &code)
    {
        create(device, code);
    }

vulkanShaderModule::~vulkanShaderModule()
    {
        cleanup();
    }

void vulkanShaderModule::create(VkDevice device, const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device, &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanShaderModule::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
        m_shaderModule = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

bool vulkanShaderModule::isCreated() const
    {
        return !m_cleanedUp;
    }

VkShaderModule vulkanShaderModule::getUnderlyingShaderModule() const
    {
        return m_shaderModule;
    }

vulkanShaderModule::operator VkShaderModule() const
    {
        return m_shaderModule;
    }

VkShaderModule &vulkanShaderModule::getUnderlyingShaderModule()
    {
        return m_shaderModule;
    }
