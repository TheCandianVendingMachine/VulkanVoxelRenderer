#include "voxel/voxelSpace.hpp"
#include "graphics/vertex.hpp"
#include "graphics/quad.hpp"
#include "graphics/descriptorSet.hpp"
#include "typeDefines.hpp"
#include "PerlinNoise.hpp"
#include "random.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include "graphics/vulkan/vulkanCommandBuffer.hpp"
#include <vk_mem_alloc.h>
#include <vector>
#include <array>
#include <glm/gtx/quaternion.hpp>

void voxelSpace::updateChunkMemory(chunkData &chunk, void *stagingBuffer, unsigned long long vertexBufferOffset)
    {
        unsigned int vertexOffset = 0;
        unsigned int indexOffset = 0;

        fe::uInt8 *cpuStagingBuffer = static_cast<fe::uInt8*>(m_localBuffer.m_cpuStagingBuffer);
        for (auto &voxelData : chunk.m_voxelData)
            {
                unsigned long long indexSize = voxelData.m_indices.size() * sizeof(fe::index);
                unsigned long long vertexSize = voxelData.m_vertices.size() * sizeof(vertex);

                std::memcpy(cpuStagingBuffer + indexOffset, voxelData.m_indices.data(), indexSize);
                std::memcpy(cpuStagingBuffer + vertexBufferOffset + vertexOffset, voxelData.m_vertices.data(), vertexSize);

                indexOffset += indexSize;
                vertexOffset += vertexSize;
            }
    }

void voxelSpace::updateMemory()
    {
        unsigned int totalVertexCount = m_testChunk.m_vertexCount;
        unsigned int totalIndexCount = m_testChunk.m_indexCount;

        if (m_localBuffer.m_maxVertexCount < totalVertexCount || m_localBuffer.m_maxIndexCount < totalIndexCount)
            {
                m_localBuffer.create(totalVertexCount, totalIndexCount);
            }

        // for all chunks
        updateChunkMemory(m_testChunk, static_cast<fe::int8*>(m_localBuffer.m_cpuStagingBuffer) + 0, getVertexMemoryOffset());
    }

void voxelSpace::createChunk(chunkData &chunk, const voxelChunk::sizeType sizeX, const voxelChunk::sizeType sizeY, const voxelChunk::sizeType sizeZ)
    {
        chunk.m_chunk.create(sizeX, sizeY, sizeZ);

        // We are getting the best number to subdivide our chunk to the threshold we want
        // We over-estimate the actual size so we dont leave out data. I would rather extra than too few
        voxelChunk::sizeType subChunkCountX = (sizeX + (c_chunkSubSize - (sizeX % c_chunkSubSize))) / c_chunkSubSize;
        voxelChunk::sizeType subChunkCountY = (sizeY + (c_chunkSubSize - (sizeY % c_chunkSubSize))) / c_chunkSubSize;
        voxelChunk::sizeType subChunkCountZ = (sizeZ + (c_chunkSubSize - (sizeZ % c_chunkSubSize))) / c_chunkSubSize;

        chunk.m_voxelData.resize(static_cast<std::size_t>(subChunkCountX * subChunkCountY * subChunkCountZ));
        chunk.m_subSize = c_chunkSubSize;

        chunk.m_sizeX = sizeX;
        chunk.m_sizeY = sizeY;
        chunk.m_sizeZ = sizeZ;
    }

void voxelSpace::buildChunk(chunkData &chunk)
    {
        chunk.m_vertexCount = 0;
        chunk.m_indexCount = 0;

        unsigned int totalIndexOffset = 0;

        buildChunkMesh(chunk);
        for (auto &subChunk : chunk.m_voxelData)
            {
                totalIndexOffset = buildGeometry(subChunk, totalIndexOffset);
                chunk.m_vertexCount += subChunk.m_vertices.size();
                chunk.m_indexCount += subChunk.m_indices.size();
            }
    }

void voxelSpace::destroyChunk(chunkData &chunk)
    {
        for (auto &voxelData : chunk.m_voxelData)
            {
                voxelData.m_quads.clear();
            }
    }

voxelSpace::~voxelSpace()
    {
        destroy();
    }

void voxelSpace::create()
    {
    }

void voxelSpace::destroy()
    {
        destroyChunk(m_testChunk);
        m_localBuffer.destroy();
    }

std::vector<float> noiseMapGenerator(int sizeX, int sizeY, int sizeZ, int seed, float scale, int octaves, float persistance, float lacunarity, glm::vec3 offset)
    {
        std::vector<float> noise(sizeX * sizeY * sizeZ);
        const siv::PerlinNoise noiseGenerator(seed);

        std::vector<glm::vec3> octaveOffsets(octaves);
        for (int i = 0; i < octaves; octaves++)
            {
                float offsetX = fe::random::get().generate(-100000.f, 100000.f) + offset.x;
                float offsetY = fe::random::get().generate(-100000.f, 100000.f) + offset.y;
                float offsetZ = fe::random::get().generate(-100000.f, 100000.f) + offset.z;

                octaveOffsets[i] = { offsetX, offsetY, offsetZ };
            }

        if (scale <= 0.f)
            {
                scale = 0.000000001f;
            }

        float maxNoiseHeight = std::numeric_limits<float>::min();
        float minNoiseHeight = std::numeric_limits<float>::max();

        float halfWidth = static_cast<float>(sizeX) / 2.f;
        float halfHeight = static_cast<float>(sizeY) / 2.f;
        float halfDepth = static_cast<float>(sizeZ) / 2.f;

        for (int y = 0; y < sizeY; y++)
            {
                for (int z = 0; z < sizeZ; z++)
                    {
                        for (int x = 0; x < sizeX; x++)
                            {
                                float amplitude = 1.f;
                                float frequency = 1.f;
                                float noiseHeight = 0.f;

                                for (int i = 0; i < octaves; i++)
                                    {
                                        float sampleX = (x - halfWidth) / scale * frequency + octaveOffsets[i].x;
                                        float sampleY = (y - halfHeight) / scale * frequency + octaveOffsets[i].y;
                                        float sampleZ = (z - halfDepth) / scale * frequency + octaveOffsets[i].z;

                                        float perlinValue = static_cast<float>(noiseGenerator.noise3D(sampleX, sampleY, sampleZ));
                                        noiseHeight += perlinValue * amplitude;

                                        amplitude += persistance;
                                        frequency += lacunarity;
                                    }

                                if (noiseHeight > maxNoiseHeight)
                                    {
                                        maxNoiseHeight = noiseHeight;
                                    }
                                if (noiseHeight < minNoiseHeight)
                                    {
                                        minNoiseHeight = noiseHeight;
                                    }

                                noise[x + sizeX * (y + sizeZ * z)] = noiseHeight;
                            }
                    }
            }

        for (int y = 0; y < sizeY; y++)
            {
                for (int z = 0; z < sizeZ; z++)
                    {
                        for (int x = 0; x < sizeX; x++)
                            {
                                float currentNoise = noise[x + sizeX * (y + sizeZ * z)];
                                // inverse lerp: (v - a) / (a - b)
                                currentNoise = (currentNoise - minNoiseHeight) / (maxNoiseHeight - minNoiseHeight);

                                noise[x + sizeX * (y + sizeZ * z)] = currentNoise;
                            }
                    }
            }

        return noise;
    }

void voxelSpace::createWorld()
    {
        double globalFrequency = 0.85;
        std::array<std::array<double, 2>, 2> surfaceFrequencies = {{
            {{ 1.0, 1.5 }},
            {{ 0.2, 4.0 }},
        }};

        const siv::PerlinNoise noiseSurface(fe::random::get().generate<uint32_t>());
        createChunk(m_testChunk, 48, 48, 48);

        for (int x = 0; x < m_testChunk.m_chunk.getSizeX(); x++)
            {
                double nx = (static_cast<double>(x) / m_testChunk.m_chunk.getSizeX());
                for (int y = 0; y < m_testChunk.m_chunk.getSizeY(); y++)
                    {
                        double ny = (static_cast<double>(y) / m_testChunk.m_chunk.getSizeY());
                        for (int z = 0; z < m_testChunk.m_chunk.getSizeZ(); z++)
                            {
                                double nz = (static_cast<double>(z) / m_testChunk.m_chunk.getSizeZ());
                                double surfaceNoise = 0.0;
                                for (const auto &frequency : surfaceFrequencies)
                                    {
                                        surfaceNoise += globalFrequency * frequency[0] * noiseSurface.noise3D_0_1(frequency[1] * nx, frequency[1] * ny, frequency[1] * nz);
                                    }

                                voxelType type = voxelType::NONE;
                                if (surfaceNoise < 0.5)
                                    {
                                        type = voxelType::DEFAULT;
                                    }
                                m_testChunk.m_chunk.at(x, y, z) = type;
                            }
                    }
            }

        buildChunk(m_testChunk);
        updateMemory();

        m_translation = glm::translate(glm::mat4(1.f), glm::vec3{0.f, 0.f, 0.f});
        m_quaternion = glm::angleAxis(0.f, glm::normalize(glm::vec3{ 0, 1.f, 0.f }));
    }

glm::mat4 voxelSpace::getModelTransformation() const
    {
        return m_translation * glm::toMat4(m_quaternion);
    }

glm::vec<3, int> voxelSpace::raycast(const glm::vec3 origin, const glm::vec3 direction)
    {
        constexpr float gridOffset = 0.000001f;

        glm::vec3 worldPosition = getModelTransformation() * glm::vec4(origin, 0.f);
        glm::vec<3, int> gridPosition = worldPosition / m_testChunk.m_chunk.getVoxelSize();

        glm::vec3 deltaDistance = glm::abs(1.f / direction);
        glm::vec3 sideDistance;
        glm::vec<3, int> step;

        if (direction.x < 0.f)
            {
                step.x = -1;
                sideDistance.x = (worldPosition.x - gridPosition.x) * deltaDistance.x;
            }
        else
            {
                step.x = 1;
                sideDistance.x = (gridPosition.x + gridOffset - worldPosition.x) * deltaDistance.x;
            }

        if (direction.y < 0.f)
            {
                step.y = -1;
                sideDistance.y = (worldPosition.y - gridPosition.y) * deltaDistance.y;
            }
        else
            {
                step.y = 1;
                sideDistance.y = (gridPosition.y + gridOffset - worldPosition.y) * deltaDistance.y;
            }

        if (direction.z < 0.f)
            {
                step.z = -1;
                sideDistance.z = (worldPosition.z - gridPosition.z) * deltaDistance.z;
            }
        else
            {
                step.z = 1;
                sideDistance.z = (gridPosition.z + gridOffset - worldPosition.z) * deltaDistance.z;
            }

        int stepCount = 0;
        const int maxStepCount = static_cast<int>(m_testChunk.m_chunk.getSizeX() + m_testChunk.m_chunk.getSizeY() + m_testChunk.m_chunk.getSizeZ());
        while (stepCount++ <= maxStepCount)
            {
                if (sideDistance.x < sideDistance.z)
                    {
                        if (sideDistance.y < sideDistance.x)
                            {
                                sideDistance.y += deltaDistance.y;
                                gridPosition.y += step.y;
                            }
                        else
                            {
                                sideDistance.x += deltaDistance.x;
                                gridPosition.x += step.x;
                            }
                    }
                else
                    {
                        if (sideDistance.y < sideDistance.z)
                            {
                                sideDistance.y += deltaDistance.y;
                                gridPosition.y += step.y;
                            }
                        else
                            {
                                sideDistance.z += deltaDistance.z;
                                gridPosition.z += step.z;
                            }
                    }

                if (m_testChunk.m_chunk.withinBounds(gridPosition.x, gridPosition.y, gridPosition.z))
                    {
                        voxelType type = m_testChunk.m_chunk.at(gridPosition.x, gridPosition.y, gridPosition.z);
                        if (type != voxelType::NONE)
                            {
                                return gridPosition;
                            }
                    }
            }

        return {};
    }

void voxelSpace::updateBuffers(vulkanCommandBuffer &commandBuffer)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = m_localBuffer.m_bufferSize;
        vkCmdCopyBuffer(commandBuffer, m_localBuffer.m_masterStagingBuffer, m_localBuffer.m_masterBuffer, 1, &copyRegion);
        m_localBuffer.m_needUpdate = false;
    }

bool voxelSpace::needsUpdate() const
    {
        return m_localBuffer.m_needUpdate;
    }

vulkanBuffer &voxelSpace::getBufferMemory()
    {
        return m_localBuffer.m_masterBuffer;
    }

VkDeviceSize voxelSpace::getVertexMemoryOffset() const
    {
        return m_localBuffer.m_maxIndexCount * sizeof(fe::index);
    }

VkDeviceSize voxelSpace::getIndexBufferSize() const
    {
        return getIndexCount() * sizeof(fe::index);
    }

VkDeviceSize voxelSpace::getVertexBufferSize() const
    {
        return getVertexCount() * sizeof(vertex);
    }

unsigned int voxelSpace::getVertexCount() const
    {
        unsigned int vertexCount = 0;
        vertexCount += m_testChunk.m_vertexCount;

        return vertexCount;
    }

unsigned int voxelSpace::getIndexCount() const
    {
        unsigned int indexCount = 0;
        indexCount += m_testChunk.m_indexCount;

        return indexCount;
    }

void buildChunkMesh(voxelSpace::chunkData &chunkData)
    {
        voxelChunk::sizeType x = 0;
        voxelChunk::sizeType y = 0;
        voxelChunk::sizeType z = 0;
        for (auto &voxelData : chunkData.m_voxelData)
            {
                for (voxelChunk::sizeType yIncrement = 0; yIncrement < chunkData.m_subSize; yIncrement++)
                    {
                        for (voxelChunk::sizeType xIncrement = 0; xIncrement < chunkData.m_subSize; xIncrement++)
                            {
                                for (voxelChunk::sizeType zIncrement = 0; zIncrement < chunkData.m_subSize; zIncrement++)
                                    {
                                        chunkData.m_chunk.meshAtPosition(voxelData.m_quads, x + xIncrement, y + yIncrement, z + zIncrement);
                                    }
                            }
                    }

                x += chunkData.m_subSize;
                if (x > chunkData.m_sizeX)
                    {
                        x = 0;
                        y += chunkData.m_subSize;
                        if (y > chunkData.m_sizeY)
                            {
                                y = 0;
                                z += chunkData.m_subSize;
                            }
                    }               
            }
    }

unsigned int buildGeometry(voxelSpace::chunkVoxelData &voxelData, unsigned int indexOffset)
    {
        vertex vertexArr[4] = {};
        vertex &v0 = vertexArr[0];
        vertex &v1 = vertexArr[1];
        vertex &v2 = vertexArr[2];
        vertex &v3 = vertexArr[3];

        fe::index indexArrPositive[6] = {
            0, 1, 2, 0, 2, 3
        };
        fe::index indexArrNegative[6] = {
            2, 1, 0, 3, 2, 0
        };

        indexArrPositive[0] += indexOffset;
        indexArrPositive[1] += indexOffset;
        indexArrPositive[2] += indexOffset;
        indexArrPositive[3] += indexOffset;
        indexArrPositive[4] += indexOffset;
        indexArrPositive[5] += indexOffset;

        indexArrNegative[0] += indexOffset;
        indexArrNegative[1] += indexOffset;
        indexArrNegative[2] += indexOffset;
        indexArrNegative[3] += indexOffset;
        indexArrNegative[4] += indexOffset;
        indexArrNegative[5] += indexOffset;

        glm::vec3 colours[] = {
            { 0.23f, 0.48f, 0.34f },
            { 0.53f, 0.81f, 0.92f },
            { 0.37f, 0.50f, 0.22f }
        };

        for (auto &quad : voxelData.m_quads)
            {
                v0.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];
                v1.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];
                v2.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];
                v3.m_colour = quad.m_colour * colours[std::abs(quad.m_orientation) - 1];

                switch (std::abs(quad.m_orientation))
                    {
                        case 1:
                            v0.m_position.x = quad.m_position.x;
                            v0.m_position.y = quad.m_position.y;
                            v0.m_position.z = quad.m_position.z;

                            v1.m_position.x = quad.m_position.x + quad.m_size.x;
                            v1.m_position.y = quad.m_position.y;
                            v1.m_position.z = quad.m_position.z;

                            v2.m_position.x = quad.m_position.x + quad.m_size.x;
                            v2.m_position.y = quad.m_position.y + quad.m_size.y;
                            v2.m_position.z = quad.m_position.z;

                            v3.m_position.x = quad.m_position.x;
                            v3.m_position.y = quad.m_position.y + quad.m_size.y;
                            v3.m_position.z = quad.m_position.z;
                            break;
                        case 2:
                            v0.m_position.x = quad.m_position.x;
                            v0.m_position.y = quad.m_position.z;
                            v0.m_position.z = quad.m_position.y;

                            v1.m_position.x = quad.m_position.x + quad.m_size.x;
                            v1.m_position.y = quad.m_position.z;
                            v1.m_position.z = quad.m_position.y;

                            v2.m_position.x = quad.m_position.x + quad.m_size.x;
                            v2.m_position.y = quad.m_position.z;
                            v2.m_position.z = quad.m_position.y + quad.m_size.y;

                            v3.m_position.x = quad.m_position.x;
                            v3.m_position.y = quad.m_position.z;
                            v3.m_position.z = quad.m_position.y + quad.m_size.y;
                            break;
                        case 3:
                            v0.m_position.x = quad.m_position.z;
                            v0.m_position.y = quad.m_position.x;
                            v0.m_position.z = quad.m_position.y;

                            v1.m_position.x = quad.m_position.z;
                            v1.m_position.y = quad.m_position.x + quad.m_size.x;
                            v1.m_position.z = quad.m_position.y;

                            v2.m_position.x = quad.m_position.z;
                            v2.m_position.y = quad.m_position.x + quad.m_size.x;
                            v2.m_position.z = quad.m_position.y + quad.m_size.y;

                            v3.m_position.x = quad.m_position.z;
                            v3.m_position.y = quad.m_position.x;
                            v3.m_position.z = quad.m_position.y + quad.m_size.y;
                            break;
                        default:
                            break;
                    }

                voxelData.m_vertices.insert(voxelData.m_vertices.end(), vertexArr, vertexArr + 4);
                if (quad.m_orientation < 0)
                    {
                        voxelData.m_indices.insert(voxelData.m_indices.end(), indexArrNegative, indexArrNegative + 6);
                    }
                else 
                    {
                        voxelData.m_indices.insert(voxelData.m_indices.end(), indexArrPositive, indexArrPositive + 6);
                    }
                
                indexArrPositive[0] += 4;
                indexArrPositive[1] += 4;
                indexArrPositive[2] += 4;
                indexArrPositive[3] += 4;
                indexArrPositive[4] += 4;
                indexArrPositive[5] += 4;

                indexArrNegative[0] += 4;
                indexArrNegative[1] += 4;
                indexArrNegative[2] += 4;
                indexArrNegative[3] += 4;
                indexArrNegative[4] += 4;
                indexArrNegative[5] += 4;
            }

        return indexOffset + voxelData.m_vertices.size();
    }

void voxelSpace::localBuffer::create(unsigned int vertexCount, unsigned int indexCount)
    {
        destroy();

        m_bufferSize = vertexCount * sizeof(vertex) + indexCount * sizeof(fe::index);
        m_masterStagingBuffer.create(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_COPY);

        VmaMemoryUsage bufferUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        m_masterBuffer.create(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferUsage);

        m_maxIndexCount = indexCount;
        m_maxVertexCount = vertexCount;

        vmaMapMemory(*globals::g_vulkanAllocator, m_masterStagingBuffer.getUnderlyingAllocation(), &m_cpuStagingBuffer);
        m_needUpdate = true;
        m_exists = true;
    }

void voxelSpace::localBuffer::destroy()
    {
        if (!m_exists) { return; }

        m_bufferSize = 0;
        m_maxIndexCount = 0;
        m_maxVertexCount = 0;

        vmaUnmapMemory(*globals::g_vulkanAllocator, m_masterStagingBuffer.getUnderlyingAllocation());

        m_masterStagingBuffer.cleanup();
        m_masterBuffer.cleanup();
        m_exists = false;
    }
