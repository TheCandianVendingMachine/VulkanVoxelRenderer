// raytracer.hpp
// Ray traces a voxel scene. Owns the compute shader and handles shader resources
#pragma once
#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"
#include "graphics/uniformBuffer.hpp"
#include "graphics/descriptorSet.hpp"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class heightmap;
class renderer;
class raytracer
    {
        private:
            heightmap *m_heightmap = nullptr;
            renderer *m_renderer = nullptr;

            glm::ivec2 m_imageSize;

            static constexpr unsigned int c_computeThreads = 32;
            static constexpr unsigned int c_maxGroundTextures = 4;
            unsigned int m_computePipelineID = -1;
            descriptorSet *m_computeDescriptors = nullptr;

            alignas(16) vulkanImage m_renderImage;
            vulkanImageView m_renderImageView;

            alignas(16) vulkanImage m_skySphere;
            vulkanImageView m_skySphereView;
            vulkanSampler m_skySphereSampler;

            alignas(16) vulkanImage m_groundTextures[c_maxGroundTextures];
            vulkanImageView m_groundTexturesViews[c_maxGroundTextures];
            vulkanSampler m_groundTexturesSamplers[c_maxGroundTextures];

            alignas(16) vulkanImage m_finalImage;
            vulkanImageView m_finalImageView;
            vulkanSampler m_finalImageSampler;
            descriptorSet *m_fragDescriptors = nullptr;

            uniformBuffer m_groundTerrainDataUBO;
            uniformBuffer m_heightmapVariablesUBO;
            uniformBuffer m_shaderVariablesUBO;

            struct
                {
                    int m_maxTraces = 3;
                    float m_normalEpsilon = 16.f;
                    bool m_displayNormals = false;
                } m_shaderVariables;
            
            struct
                {
                    int m_minCheckDistance = 15;
                    float m_minT = 0.05f;
                    float m_maxT = 25000.f;
                    float m_tIncrement = 2.f;
                    float m_softShadowScale = 128.f;
                } m_heightmapVariables;

            struct
                {
                    int m_groundTextureCount = 0;
                    glm::vec3 m_heights[c_maxGroundTextures] = {}; // vec3 because for some reason vec2's don't send all the bytes to GPU
                } m_groundTextureData;

            void initComputePipeline(renderer &renderer, heightmap &heightmap, uniformBuffer &viewUBO, uniformBuffer &lightUBO, vulkanImageView &voxelGridView, vulkanImageView &voxelShadowGridView, vulkanSampler &voxelGridSampler, vulkanSampler &voxelShadowGridSampler);

        public:
            raytracer() = default;
            raytracer(renderer &renderer, heightmap &heightmap, glm::ivec2 imageSize, const char *skySphere, uniformBuffer &viewUBO, uniformBuffer &lightUBO, vulkanImageView &voxelGridView, vulkanImageView &voxelShadowGridView, vulkanSampler &voxelGridSampler, vulkanSampler &voxelShadowGridSampler);
            ~raytracer();
            void create(renderer &renderer, heightmap &heightmap, glm::ivec2 imageSize, const char *skySphere, uniformBuffer &viewUBO, uniformBuffer &lightUBO, vulkanImageView &voxelGridView, vulkanImageView &voxelShadowGridView, vulkanSampler &voxelGridSampler, vulkanSampler &voxelShadowGridSampler);
            void destroy();

            void addGroundTexture(const char *filepath, float minHeight, float maxHeight);

            void dispatch();
            void draw(bool debug);
    };
