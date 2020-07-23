#include "graphics/shader.hpp"
#include <fstream>

void shader::load(VkDevice device, const std::string &filepath, const std::string &entryPoint)
    {
        m_entry = entryPoint;

        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            {
                throw std::runtime_error("failed to open file!");
            }

        std::size_t fileSize = (std::size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        m_shader.create(device, buffer);
    }

const std::string &shader::getEntryPoint() const
    {
        return m_entry;
    }

const vulkanShaderModule &shader::getShaderModule() const
    {
        return m_shader;
    }
