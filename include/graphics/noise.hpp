// noise.hpp
// Generates a noise texture and allows binding to shaders
#pragma once
#include "FastNoise.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"

class renderer;
class noise
    {
        private:
            FastNoise m_noiseGenerator;
            glm::uvec3 m_noiseTextureSize = { 0, 0, 0 };

            alignas(16) vulkanImage m_noiseImage;
            vulkanImageView m_noiseView;
            vulkanSampler m_noiseSampler;

            void generateTexture(bool threeDimensions, renderer &renderer);

        public:
            noise();
            noise(FastNoise::NoiseType noiseType, FN_DECIMAL frequency, FN_DECIMAL lacunarity, FN_DECIMAL gain, int octaves);
            void init(FastNoise::NoiseType noiseType, FN_DECIMAL frequency, FN_DECIMAL lacunarity, FN_DECIMAL gain, int octaves);

            ~noise();
            void destroy();

            void createTexture(glm::uvec2 size, renderer &renderer);
            void createTexture(glm::uvec3 size, renderer &renderer);

            FN_DECIMAL sample(FN_DECIMAL x, FN_DECIMAL y) const;
            FN_DECIMAL sample(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;

            const vulkanImage &getImage() const;
            const vulkanSampler &getSampler() const;
            const vulkanImageView &getView() const;
    };