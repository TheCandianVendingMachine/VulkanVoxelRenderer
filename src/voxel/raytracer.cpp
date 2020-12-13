#include "voxel/raytracer.hpp"
#include "voxel/heightmap.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptorSettings.hpp"
#include "graphics/textureFunctions.hpp"
#include "voxel/voxelGrid.hpp"

#include "imgui.h"

void raytracer::initComputePipeline(renderer &renderer, heightmap &heightmap, uniformBuffer &viewUBO, uniformBuffer &lightUBO, voxelGrid &grid)
    {
        descriptorSettings computeSettings;
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // output
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // camera
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // light
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // voxels
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // voxel shadows
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // heightmap
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, c_maxGroundTextures); // ground texture samplers
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // ground texture data
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // skybox
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // heightmap variables
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // shader variables
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1); // grid variables
        
        m_computePipelineID = renderer.createComputePipeline(computeSettings, "shaders/voxel_raytracing_gl.spv");

        m_groundTerrainDataUBO.createAndBind(m_groundTextureData);
        m_heightmapVariablesUBO.createAndBind(m_heightmapVariables);
        m_shaderVariablesUBO.createAndBind(m_shaderVariables);
        m_gridVariablesUBO.createAndBind(m_gridVariables);

        m_computeDescriptors = renderer.createComputeDescriptorSet(m_computePipelineID);
        m_computeDescriptors->bindImage(m_renderImageView, 0);
        m_computeDescriptors->bindUBO(viewUBO.getUniformBuffer(), viewUBO.getBufferSize(), 1);
        m_computeDescriptors->bindUBO(lightUBO.getUniformBuffer(), lightUBO.getBufferSize(), 2);
        m_computeDescriptors->bindImage(grid.m_gridView, grid.m_gridSampler, 3);
        m_computeDescriptors->bindImage(grid.m_shadowGridView, grid.m_shadowGridSampler, 4);
        m_computeDescriptors->bindImage(heightmap.getView(), heightmap.getSampler(), 5);
        m_computeDescriptors->bindImages(m_groundTexturesViews, m_groundTexturesSamplers, m_groundTextureData.m_groundTextureCount, 6);
        m_computeDescriptors->bindUBO(m_groundTerrainDataUBO.getUniformBuffer(), m_groundTerrainDataUBO.getBufferSize(), 7);
        m_computeDescriptors->bindImage(m_skySphereView, m_skySphereSampler, 8);
        m_computeDescriptors->bindUBO(m_heightmapVariablesUBO.getUniformBuffer(), m_heightmapVariablesUBO.getBufferSize(), 9);
        m_computeDescriptors->bindUBO(m_shaderVariablesUBO.getUniformBuffer(), m_shaderVariablesUBO.getBufferSize(), 10);
        m_computeDescriptors->bindUBO(m_gridVariablesUBO.getUniformBuffer(), m_gridVariablesUBO.getBufferSize(), 11);
    }

raytracer::raytracer(renderer &renderer, heightmap &heightmap, glm::ivec2 imageSize, const char *skySphere, uniformBuffer &viewUBO, uniformBuffer &lightUBO, voxelGrid &grid)
    {
        create(renderer, heightmap, imageSize, skySphere, viewUBO, lightUBO, grid);
    }

raytracer::~raytracer()
    {
        destroy();
    }

void raytracer::create(renderer &renderer, heightmap &heightmap, glm::ivec2 imageSize, const char *skySphere, uniformBuffer &viewUBO, uniformBuffer &lightUBO, voxelGrid &grid)
    {
        m_renderImage.create(imageSize.x, imageSize.y, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VkImageType::VK_IMAGE_TYPE_2D);
        m_renderImageView.create(renderer.getDevice(), m_renderImage, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D);
        renderer.transitionImageLayout(m_renderImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        loadTexture(skySphere, m_skySphere, renderer);
        m_skySphereView.create(renderer.getDevice(), m_skySphere, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_skySphere.mipLevels, VK_IMAGE_VIEW_TYPE_2D);
        m_skySphereSampler.create(renderer.getDevice(), m_skySphere.mipLevels, VK_SAMPLER_ADDRESS_MODE_REPEAT);

        initComputePipeline(renderer, heightmap, viewUBO, lightUBO, grid);

        m_finalImage.create(imageSize.x, imageSize.y, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VkImageType::VK_IMAGE_TYPE_2D);
        m_finalImageView.create(renderer.getDevice(), m_finalImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D);
        m_finalImageSampler.create(renderer.getDevice(), m_finalImage.mipLevels, VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        renderer.transitionImageLayout(m_finalImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_fragDescriptors = renderer.createDescriptorSet();
        m_fragDescriptors->bindImage(m_finalImageView, m_finalImageSampler, 0);

        m_heightmap = &heightmap;
        m_renderer = &renderer;
        m_imageSize = imageSize;
    }

void raytracer::destroy()
    {
        for (unsigned int i = 0; i < m_groundTextureData.m_groundTextureCount; i++)
            {
                m_groundTexturesSamplers[i].cleanup();
                m_groundTexturesViews[i].cleanup();
                m_groundTextures[i].cleanup();
            }

        m_skySphereSampler.cleanup();
        m_skySphereView.cleanup();
        m_skySphere.cleanup();

        m_renderImageView.cleanup();
        m_renderImage.cleanup();

        m_finalImageSampler.cleanup();
        m_finalImageView.cleanup();
        m_finalImage.cleanup();

        m_groundTerrainDataUBO.destroy();
        m_shaderVariablesUBO.destroy();
        m_heightmapVariablesUBO.destroy();
        m_gridVariablesUBO.destroy();
    }

void raytracer::addGroundTexture(const char *filepath, float minHeight, float maxHeight)
    {
        unsigned int index = m_groundTextureData.m_groundTextureCount;
        if (index >= c_maxGroundTextures)
            {
                // <error>
                return;
            }

        loadTexture(filepath, m_groundTextures[index], *m_renderer);
        m_groundTexturesViews[index].create(m_renderer->getDevice(), m_groundTextures[index], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_groundTextures[index].mipLevels, VK_IMAGE_VIEW_TYPE_2D);
        m_groundTexturesSamplers[index].create(m_renderer->getDevice(), m_groundTextures[index].mipLevels, VK_SAMPLER_ADDRESS_MODE_REPEAT);

        m_groundTextureData.m_heights[index].x = minHeight;
        m_groundTextureData.m_heights[index].y = maxHeight;

        m_groundTextureData.m_groundTextureCount++;

        m_groundTerrainDataUBO.bind(m_groundTextureData);
        m_computeDescriptors->bindImages(m_groundTexturesViews, m_groundTexturesSamplers, m_groundTextureData.m_groundTextureCount, 6);
    }

void raytracer::dispatch()
    {
        if (m_computeDescriptors->needsUpdate())
            {
                m_computeDescriptors->update();
            }
        m_renderer->dispatchCompute(m_computePipelineID, m_computeDescriptors, m_imageSize.x / c_computeThreads, m_imageSize.y / c_computeThreads, 1);
    }

void raytracer::draw(bool debug)
    {
        m_renderer->blitImage(m_renderImage, VK_IMAGE_LAYOUT_GENERAL, m_finalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_imageSize.x, m_imageSize.y);
        m_renderer->draw(*m_fragDescriptors);

        if (debug)
            {
                ImGui::Begin("Raycast Settings");

                ImGui::Text("Shader Variables");
                bool updateUBO = false;
                updateUBO |= ImGui::Selectable("Draw Normals", &m_shaderVariables.m_displayNormals);
                updateUBO |= ImGui::SliderInt("Max Traces", &m_shaderVariables.m_maxTraces, 0, 8);
                updateUBO |= ImGui::SliderFloat("Normal Epsilon", &m_shaderVariables.m_normalEpsilon, 0.001f, 64.f);

                if (updateUBO)
                    {
                        m_shaderVariablesUBO.bind(m_shaderVariables);
                    }
                updateUBO = false;
                ImGui::NewLine();
                ImGui::Text("Heightmap Raymarch Variables");
                updateUBO |= ImGui::SliderInt("Min Check Distance", &m_heightmapVariables.m_minCheckDistance, 1, 100);
                updateUBO |= ImGui::SliderFloat("March Increment", &m_heightmapVariables.m_tIncrement, 0.1f, 10.f);
                updateUBO |= ImGui::SliderFloat("Min Ray Distance", &m_heightmapVariables.m_minT, 0.01f, 5.f);
                updateUBO |= ImGui::SliderFloat("Max Ray Distance", &m_heightmapVariables.m_maxT, 1.f, 50000.f, "%.3f", 2.f);
                updateUBO |= ImGui::SliderFloat("Soft Shadow Scale", &m_heightmapVariables.m_softShadowScale, 0.01f, 256.f);
                if (updateUBO)
                    {
                        m_heightmapVariablesUBO.bind(m_heightmapVariables);
                    }

                updateUBO = false;
                ImGui::NewLine();
                ImGui::Text("Grid Variables");
                updateUBO |= ImGui::SliderInt("MIP level", &m_gridVariables.m_mip, 0, voxelGrid::c_maxDepth);
                if (updateUBO)
                    {
                        m_gridVariablesUBO.bind(m_gridVariables);
                    }

                ImGui::End();
            }
    }
