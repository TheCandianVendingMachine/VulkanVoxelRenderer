// heightmap.hpp
// Represents a heightmap image. Can get distance to points and whatnot
#pragma once
#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"
#include <stb_image.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class renderer;
class heightmap
    {
        private:
            unsigned char *m_pixelData = nullptr;
            int m_channels = 0;
            glm::ivec2 m_size;

            int m_mapWidthScale = 16; // x Meters / 1 Resolution
            int m_mapHeightScale = 64; // x Meters / 1 Meter

            alignas(16) vulkanImage m_heightmap;
            vulkanImageView m_heightmapView;
            vulkanSampler m_heightmapSampler;

        public:
            heightmap() = default;
            heightmap(const char *filepath, renderer &renderer);
            void load(const char *filepath, renderer &renderer);

            ~heightmap();
            void destroy();

            vulkanImage &getImage();
            vulkanImageView &getView();
            vulkanSampler &getSampler();

            float getHeight(glm::vec3 position);
            glm::ivec2 getSize() const;

    };
