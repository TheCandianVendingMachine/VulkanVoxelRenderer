// shader.hpp
// Shader binary data. Pre-compiled SPIR-V files are loaded and the blob creates a vulkanShaderModule
#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "graphics/vulkan/vulkanShaderModule.hpp"

class shader
    {
        private:
            vulkanShaderModule m_shader;
            std::string m_entry = "main";

        public:
            void load(VkDevice device, const std::string &filepath, const std::string &entryPoint = "main");
            const std::string &getEntryPoint() const;
            const vulkanShaderModule &getShaderModule() const;

    };
