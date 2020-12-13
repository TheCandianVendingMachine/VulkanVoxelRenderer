#include "voxel/voxelGrid.hpp"
#include "graphics/storageBuffer.hpp"
#include "graphics/renderer.hpp"
#include <fstream>
#include <glm/geometric.hpp>
#include <thread>
#include <array>

constexpr voxelGrid::indexType convertPositionToIndexF(voxelGrid::sizeTypeVec position, voxelGrid::sizeTypeVec size) noexcept
    {
        return static_cast<voxelGrid::indexType>(position.x + size.x * (position.y + size.y * position.z));
    }

constexpr voxelGrid::sizeTypeVec convertIndexToPositionF(voxelGrid::indexType index, voxelGrid::sizeTypeVec size) noexcept
    {
        const voxelGrid::indexType indexBackup = index;

        voxelGrid::sizeTypeVec position;

        position.x = index % size.x;
        index /= size.x;

        position.y = index % size.y;
        index /= size.y;

        position.z = index;

        #ifdef _DEBUG
        voxelGrid::indexType i = convertPositionToIndexF(position, size);
        assert(i == indexBackup);
        #endif
        return position;
    }

void findDistanceToNearestNeighbor(const int min, const int max, const voxelGrid::sizeTypeVec size, const std::vector<int> &occupiedVoxels, std::vector<voxelGrid::floatVoxelBinary> &floatBinaryVoxels) noexcept
    {
        for (int i = min; i < max; i++)
            {
                float nearestDistance = std::numeric_limits<float>::max();

                glm::vec3 p0 = convertIndexToPositionF(i, size);
                glm::vec3 dir;
                for (const auto &voxelIndex : occupiedVoxels)
                    {
                        glm::vec3 p1 = convertIndexToPositionF(voxelIndex, size);
                        glm::vec3 p = p1 - p0;
                        nearestDistance = std::min(nearestDistance, p.x * p.x + p.y * p.y + p.z * p.z);
                    }

                floatBinaryVoxels[i].distance = static_cast<unsigned char>(std::min(255.f, glm::sqrt(nearestDistance)));
            }
    }

constexpr voxelGrid::indexType voxelGrid::convertPositionToIndex(sizeTypeVec position) const
    {
        return convertPositionToIndexF(position, m_size);
    }

constexpr voxelGrid::sizeTypeVec voxelGrid::convertIndexToPosition(indexType index) const
    {
        return convertIndexToPositionF(index, m_size);
    }

void voxelGrid::generateMipMap(unsigned int depth, renderer &renderer)
    {
        const int finalSize = 2 << depth;
        std::vector<floatVoxel> voxelsToMap(finalSize * finalSize * finalSize);

        sizeTypeVec relativeSize = m_size / static_cast<sizeType>(finalSize >> 1);

        for (int x = 0; x < finalSize; x++)
            {
                for (int y = 0; y < finalSize; y++)
                    {
                        for (int z = 0; z < finalSize; z++)
                            {
                                if (m_octree.exists(x * relativeSize.x, y * relativeSize.y, z * relativeSize.z, depth))
                                    {
                                        auto index = convertPositionToIndexF({ x, y, z }, { finalSize, finalSize, finalSize });
                                        voxelsToMap[index].rgb = 0b1111'1111'1111'1111;
                                    }
                            }
                    }
            }

        storageBuffer buffer(voxelsToMap.size(), sizeof(floatVoxel), voxelsToMap.data());

        vulkanImage mipImage;
        mipImage.create(finalSize, finalSize, finalSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R5G6B5_UNORM_PACK16, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VkImageType::VK_IMAGE_TYPE_3D);
        renderer.transitionImageLayout(mipImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        renderer::oneTimeCommandBuffer &cb = renderer.createOneTimeBuffer();

        VkBufferImageCopy bufferCopyRegion{};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = finalSize;
        bufferCopyRegion.imageExtent.height = finalSize;
        bufferCopyRegion.imageExtent.depth = finalSize;

        vkCmdCopyBufferToImage(
            cb.m_commandBuffer.m_commandBuffer,
            buffer.getStorageBuffer(),
            mipImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &bufferCopyRegion
        );

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = mipImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            cb.m_commandBuffer.m_commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VkImageCopy copyProperties{};
        copyProperties.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyProperties.srcSubresource.baseArrayLayer = 0;
        copyProperties.srcSubresource.mipLevel = 0;
        copyProperties.srcSubresource.layerCount = 1;

        copyProperties.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyProperties.dstSubresource.baseArrayLayer = 0;
        copyProperties.dstSubresource.mipLevel = c_maxDepth - depth - 1;
        copyProperties.dstSubresource.layerCount = 1;

        copyProperties.extent.width = finalSize;
        copyProperties.extent.depth = finalSize;
        copyProperties.extent.height = finalSize;

        vkCmdCopyImage(
            cb.m_commandBuffer.m_commandBuffer,
            mipImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_gridImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &copyProperties
        );
                
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            cb.m_commandBuffer.m_commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        renderer.destroyOneTimebuffer(cb);
    }

voxelGrid::voxelGrid(sizeTypeVec size, renderer &renderer) :
    m_octree(size)
    {
        create(size, renderer);
        init(renderer);
    }

void voxelGrid::init(renderer &renderer)
    {
        m_gridImage.create(m_size.x, m_size.y, m_size.z, c_maxDepth, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R5G6B5_UNORM_PACK16, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VkImageType::VK_IMAGE_TYPE_3D);
        m_gridView.create(renderer.getDevice(), m_gridImage, VK_FORMAT_R5G6B5_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT, m_gridImage.mipLevels, VkImageViewType::VK_IMAGE_VIEW_TYPE_3D);
        m_gridSampler.create(renderer.getDevice(), m_gridImage.mipLevels, VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

        m_shadowGridImage.create(m_size.x, m_size.y, m_size.z, c_maxDepth, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VkImageType::VK_IMAGE_TYPE_3D);
        m_shadowGridView.create(renderer.getDevice(), m_shadowGridImage, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_gridImage.mipLevels, VkImageViewType::VK_IMAGE_VIEW_TYPE_3D);
        m_shadowGridSampler.create(renderer.getDevice(), m_shadowGridImage.mipLevels, VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    }

void voxelGrid::create(sizeTypeVec size, renderer &renderer)
    {
        m_size = sizeTypeVec(std::max(size.x, std::max(size.y, size.z)));
        m_voxels.resize(m_size.x * m_size.y * m_size.z);
    }

voxelGrid::~voxelGrid()
    {
        destroy();
    }

void voxelGrid::destroy()
    {
        m_shadowGridSampler.cleanup();
        m_shadowGridView.cleanup();
        m_shadowGridImage.cleanup();

        m_gridSampler.cleanup();
        m_gridView.cleanup();
        m_gridImage.cleanup();
    }

void voxelGrid::add(sizeTypeVec position, voxel voxel)
    {
        m_voxels[convertPositionToIndex(position)] = voxel;
        m_octree.addVoxel(position.x, position.y, position.z, c_maxDepth, voxel.colour.r, voxel.colour.g, voxel.colour.b);
    }

void voxelGrid::remove(sizeTypeVec position)
    {
        m_voxels[convertPositionToIndex(position)].entire = 0;
    }

void voxelGrid::mapToStorageBuffer(storageBuffer &buffer, storageBuffer &shadowBuffer)
    {
        if (!buffer.getStorageBuffer().isCreated())
            {
                buffer.create(m_gpuVoxels.size(), sizeof(floatVoxel));
            }
        buffer.bind(m_gpuVoxels.data());

        if (!shadowBuffer.getStorageBuffer().isCreated())
            {
                shadowBuffer.create(m_gpuVoxelData.size(), sizeof(floatVoxelBinary));
            }
        shadowBuffer.bind(m_gpuVoxelData.data());
    }

void voxelGrid::save(const char *file)
    {
        fileMetaData data;
        data.voxelCount = m_voxels.size();
        data.size = m_size;
        volatile int i = 0;
        std::ofstream out(file, std::ios::binary);
        if (!out.is_open())
            {
                // <error>
                return;
            }
        out.write(static_cast<const char*>(static_cast<void*>(&data)), sizeof(data));
        out.write(static_cast<const char*>(static_cast<void*>(m_voxels.data())), m_voxels.size() * sizeof(voxel));
        out.write(static_cast<const char*>(static_cast<void*>(m_gpuVoxels.data())), m_gpuVoxels.size() * sizeof(floatVoxel));
        out.write(static_cast<const char*>(static_cast<void*>(m_gpuVoxelData.data())), m_gpuVoxelData.size() * sizeof(floatVoxelBinary));
        out.close();
    }

void voxelGrid::load(const char *file)
    {
        std::ifstream in(file, std::ios::ate | std::ios::binary);
        if (!in.is_open())
            {
                // <error>
                return;
            }
        std::streampos fileSize = in.tellg();
        std::vector<char> buffer(static_cast<std::size_t>(fileSize));

        fileMetaData data;

        in.seekg(0);

        char *dataPtr = reinterpret_cast<char*>(&data);
        in.read(dataPtr, fileMetaData::c_headerMetadataSize);
        switch (data.version)
            {
                case 0:
                    // <error>
                    return;
                    break;
                case 1:
                    in.read(dataPtr + fileMetaData::c_headerMetadataSize, data.headerSize - fileMetaData::c_headerMetadataSize);
                    in.read(buffer.data(), fileSize);
                    
                    m_size = data.size;

                    m_voxels.clear();
                    m_voxels.resize(data.voxelCount);

                    m_gpuVoxels.clear();
                    m_gpuVoxels.resize(data.voxelCount);

                    m_gpuVoxelData.clear();
                    m_gpuVoxelData.resize(data.voxelCount);

                    for (std::size_t i = 0; i < m_voxels.size(); i++)
                        {
                            int offset0 = data.voxelCount * 0;
                            int offset1 = offset0 + data.voxelCount * sizeof(voxel);
                            int offset2 = offset1 + data.voxelCount * sizeof(floatVoxel);

                            m_voxels[i] = *reinterpret_cast<voxel*>(buffer.data() + offset0 + i * data.voxelSize);
                            m_gpuVoxels[i] = *reinterpret_cast<floatVoxel*>(buffer.data() + offset1 + i * data.gpuVoxelSize);
                            m_gpuVoxelData[i] = *reinterpret_cast<floatVoxelBinary*>(buffer.data() + offset2 + i * data.gpuVoxelDataSize);
                        }
                    break;
                default:
                    // <error>
                    return;
                    break;
            }
        in.close();
    }

void voxelGrid::bake()
    {
        m_gpuVoxels.resize(m_voxels.size());

        int i = 0;
        for (const auto &voxel : m_voxels)
            {
                // assuming VK_FORMAT_R5G6B5_UNORM_PACK16
                if (voxel.entire != 0)
                    {
                        m_gpuVoxels[i].rgb = voxel.colour.r | (voxel.colour.g << 5) | (voxel.colour.b << 11);
                    }
                i++;
            }

        m_gpuVoxelData.resize(m_voxels.size());
        i = 0;
        for (const auto &voxel : m_voxels)
            {   
                // assuming VK_FORMAT_R8_UNORM
                if (voxel.entire != 0)
                    {
                        m_gpuVoxelData[i].exists = 255;
                    }
                i++;
            }
    }

void voxelGrid::bakeImage(renderer &renderer)
    {
        storageBuffer temp;
        storageBuffer tempShadow;
        mapToStorageBuffer(temp, tempShadow);

        VkBufferImageCopy bufferCopyRegion{};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = m_size.x;
        bufferCopyRegion.imageExtent.height = m_size.y;
        bufferCopyRegion.imageExtent.depth = m_size.z;

        renderer.transitionImageLayout(m_gridImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(temp.getStorageBuffer(), m_gridImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

        renderer.transitionImageLayout(m_shadowGridImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(tempShadow.getStorageBuffer(), m_shadowGridImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

        // generate mipmap octree
        for (int i = 0; i < c_maxDepth - 1; i++)
            {
                generateMipMap(i, renderer);
            }

        renderer.transitionImageLayout(m_gridImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        renderer.transitionImageLayout(m_shadowGridImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        temp.destroy();
        tempShadow.destroy();
    }

bool voxelGrid::rayIntersects(glm::vec3 rayOrigin, glm::vec3 direction)
    {
        const glm::ivec3 gridSize = glm::vec3(256, 256, 256);
        const float voxelSize = 1.f;
        const glm::vec3 gridPosition = glm::vec3(1500.f, 500.f, 1500.f);

        // find point grid enters grid and set origin to adjacent voxel
        glm::vec3 workingRayOrigin = rayOrigin;
        {
            glm::vec3 q = glm::abs(workingRayOrigin - gridPosition) - glm::vec3(gridSize);
            float distanceToGrid = glm::length(glm::max(q, 0.f)) + glm::min(glm::max(glm::max(q.x, q.y), q.z), 0.f);

            if (distanceToGrid > 0.f)
            {
                workingRayOrigin = workingRayOrigin + direction * (distanceToGrid + 0.5f);
            }
        }
        glm::ivec3 origin = glm::ivec3(workingRayOrigin - gridPosition);
        glm::ivec3 step = glm::ivec3(glm::sign(direction));

        glm::vec3 tDelta = abs(1.f / direction);
        glm::vec3 tMax = (workingRayOrigin - gridPosition) * tDelta;

        while (true)
            {
                int xCond = int((tMax.x < tMax.y) && (tMax.x < tMax.z));
                int yCond = int((tMax.y < tMax.x) && (tMax.y < tMax.z));
                int zCond = int((tMax.z < tMax.x) && (tMax.z < tMax.y));

                origin += step * glm::ivec3(xCond, yCond, zCond);
                tMax += tDelta * glm::vec3(xCond, yCond, zCond);

                if (glm::any(glm::lessThan(origin, glm::ivec3(0))) || glm::any(glm::greaterThanEqual(origin, gridSize)))
                {
                    // outside grid, return immediately
                    return false;
                }

                ;
                bool hitTexel = m_gpuVoxelData[convertPositionToIndex(origin)].exists > 0;
                if (hitTexel)
                {
                    // handle outside of loop
                    float distance = glm::length(tMax);
                    return true;
                }
            }

        return false;
    }
