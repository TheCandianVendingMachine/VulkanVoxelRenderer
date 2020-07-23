// vulkanConsts.hpp
// Various constants within the Vulkan renderer
#pragma once
#include <cstdint>

namespace consts
    {
    #ifdef NDEBUG
        constexpr bool c_enableValidationLayers = false;
    #else
        constexpr bool c_enableValidationLayers = true;
    #endif

        constexpr const char *c_validationLayers[] = {
            "VK_LAYER_KHRONOS_validation",
        };
        constexpr uint32_t c_validationLayersSize = sizeof(c_validationLayers) / sizeof(c_validationLayers[0]);

        constexpr const char *c_deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        constexpr uint32_t c_deviceExtensionSize = sizeof(c_deviceExtensions) / sizeof(c_deviceExtensions[0]);
    }
