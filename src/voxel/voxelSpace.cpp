#include "voxel/voxelSpace.hpp"
#include "graphics/vertex.hpp"
#include "graphics/quad.hpp"
#include "graphics/descriptorSet.hpp"
#include "typeDefines.hpp"
#include "random.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include "graphics/vulkan/vulkanCommandBuffer.hpp"
#include <vk_mem_alloc.h>
#include <vector>
#include <array>
#include <glm/gtx/quaternion.hpp>

#include "taskGraph.hpp"
#include "task.hpp"
#include <optick.h>

void voxelSpace::updateSubChunkMemory(chunkVoxelData &voxelData)
    {
        OPTICK_EVENT();
        unsigned long long indexSize = voxelData.m_indexCount * sizeof(fe::index);
        unsigned long long vertexSize = voxelData.m_vertexCount * sizeof(vertex);

        std::memcpy(voxelData.m_indexStagingBuffer, voxelData.m_indices.data(), indexSize);
        std::memcpy(voxelData.m_vertexStagingBuffer, voxelData.m_vertices.data(), vertexSize);
        m_localBuffer.m_needUpdate = true;
    }

void voxelSpace::updateChunkMemory(chunkData &chunk)
    {
        OPTICK_EVENT();
        OPTICK_TAG("Preset Buffers | Voxel Data Count:", chunk.m_voxelData.size());
        for (auto &voxelData : chunk.m_voxelData)
            {
                updateSubChunkMemory(voxelData);
            }
        m_localBuffer.m_needUpdate = true;
    }

void voxelSpace::updateChunkMemory(chunkData &chunk, void *stagingBuffer, unsigned long long vertexBufferOffset, int &vertexOffset, int &indexOffset)
    {
        OPTICK_EVENT();
        fe::uInt8 *cpuStagingBuffer = static_cast<fe::uInt8*>(m_localBuffer.m_cpuStagingBuffer);
        for (auto &voxelData : chunk.m_voxelData)
            {
                unsigned long long indexSize = voxelData.m_indexCount * sizeof(fe::index);
                unsigned long long vertexSize = voxelData.m_vertexCount * sizeof(vertex);

                voxelData.m_indexStagingBuffer = cpuStagingBuffer + indexOffset;
                voxelData.m_vertexStagingBuffer = cpuStagingBuffer + vertexBufferOffset + vertexOffset;

                indexOffset += indexSize;
                vertexOffset += vertexSize;
            }

        updateChunkMemory(chunk);
    }

void voxelSpace::updateMemory()
    {
        OPTICK_EVENT();
        // over-allocate for worst case scenario so that we can have set memory block sizes.
        // ceil(n^3 / 2) will result in worst case voxel count. Multiply by 8 to get vertices. Multiply by 6*8 to get indices
        unsigned int totalVertexCount = getVertexCount();
        unsigned int totalIndexCount = getIndexCount();

        if (m_localBuffer.m_maxVertexCount < totalVertexCount || m_localBuffer.m_maxIndexCount < totalIndexCount)
            {
                m_localBuffer.create(totalVertexCount, totalIndexCount);
            }

        int vertexOffset = 0;
        int indexOffset = 0;
        for (auto &chunk : m_loadedChunks)
            {
                updateChunkMemory(chunk.second, static_cast<fe::int8*>(m_localBuffer.m_cpuStagingBuffer), getVertexMemoryOffset(), vertexOffset, indexOffset);
            }
        m_localBuffer.m_needUpdate = true;
    }

void voxelSpace::createChunk(chunkData &chunk, const voxelChunk::sizeType sizeX, const voxelChunk::sizeType sizeY, const voxelChunk::sizeType sizeZ, const voxelChunk::sizeType posX, const voxelChunk::sizeType posY, const voxelChunk::sizeType posZ)
    {
        chunk.m_chunk.create(sizeX, sizeY, sizeZ);
        chunk.m_chunk.setVoxelSize(c_voxelSize);

        voxelChunk::sizeType subChunkCountX = static_cast<voxelChunk::sizeType>(std::ceil(static_cast<float>(sizeX) / c_chunkSubSize));
        voxelChunk::sizeType subChunkCountY = static_cast<voxelChunk::sizeType>(std::ceil(static_cast<float>(sizeY) / c_chunkSubSize));
        voxelChunk::sizeType subChunkCountZ = static_cast<voxelChunk::sizeType>(std::ceil(static_cast<float>(sizeZ) / c_chunkSubSize));

        chunk.m_voxelData.resize(static_cast<std::size_t>(subChunkCountX * subChunkCountY * subChunkCountZ));
        chunk.m_subSize = c_chunkSubSize;

        chunk.m_sizeX = sizeX;
        chunk.m_sizeY = sizeY;
        chunk.m_sizeZ = sizeZ;

        chunk.m_positionX = posX;
        chunk.m_positionY = posY;
        chunk.m_positionZ = posZ;
    }

void voxelSpace::buildChunk(chunkData &chunk, unsigned int &totalIndexOffset)
    {
        OPTICK_CATEGORY("VoxelChunkCreation", Optick::Category::Rendering);
        OPTICK_EVENT();
        chunk.m_vertexCount = 0;
        chunk.m_indexCount = 0;

        buildChunkMesh(chunk);
        for (auto &subChunk : chunk.m_voxelData)
            {
                totalIndexOffset = buildGeometry(glm::vec3{ chunk.m_positionX, chunk.m_positionY, chunk.m_positionZ }, subChunk, totalIndexOffset);
                chunk.m_vertexCount += subChunk.m_vertexCount;
                chunk.m_indexCount += subChunk.m_indexCount;
            }
    }

void voxelSpace::destroyChunk(chunkData &chunk)
    {
        for (auto &voxelData : chunk.m_voxelData)
            {
                voxelData.m_quads.clear();
            }
    }

void voxelSpace::buildSlice(chunkData &chunk, unsigned int y, const FastNoise &noise)
    {
        for (int x = 0; x < chunk.m_chunk.getSizeX(); x++)
            {
                for (int z = 0; z < chunk.m_chunk.getSizeZ(); z++)
                    {
                        voxelType type = voxelType::NONE;
                        float surface = (1.f + noise.GetNoise(x + chunk.m_positionX, y + chunk.m_positionY, z + chunk.m_positionZ)) / 2.f;

                        if (surface < 0.5f)
                            {
                                type = voxelType::DEFAULT;
                            }
                        chunk.m_chunk.at(x, y, z) = type;
                    }
            }
    }

void voxelSpace::transformSpace(glm::vec3 globalPosition, glm::ivec3 &chunkPos, glm::vec3 &localPos) const
    {
        transformChunkSpace(globalPosition, chunkPos);
        transformLocalSpace(globalPosition, localPos);
    }

void voxelSpace::transformChunkSpace(glm::vec3 globalPosition, glm::ivec3 &chunkPos) const
    {
        chunkPos = glm::floor(globalPosition / static_cast<glm::vec3>(c_chunkSize));
    }

void voxelSpace::transformLocalSpace(glm::vec3 globalPosition, glm::vec3 &localPos) const
    {
        localPos = glm::mod(globalPosition, static_cast<glm::vec3>(c_chunkSize));
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
        for (auto &chunk : m_loadedChunks)
            {
                destroyChunk(chunk.second);
            }
        m_localBuffer.destroy();
    }

void voxelSpace::createWorld(taskGraph *graph)
    {
        FastNoise noiseSurface(fe::random::get().generate<uint32_t>());
        noiseSurface.SetNoiseType(FastNoise::NoiseType::ValueFractal);
        noiseSurface.SetFrequency(0.05f);
        noiseSurface.SetFractalOctaves(5);
        noiseSurface.SetFractalLacunarity(2.0);
        noiseSurface.SetFractalGain(0.4);

        for (int x = 0; x < 5; x++)
            {
                for (int z = 0; z < 5; z++)
                    {
                        createChunk(m_loadedChunks[glm::ivec3{ x, 0, z }], c_chunkSize.x, c_chunkSize.y, c_chunkSize.z, c_chunkSize.x * x, 0, c_chunkSize.z * z);
                    }
            }
        
        for (auto &chunk : m_loadedChunks)
            {
                for (int i = 0; i < chunk.second.m_chunk.getSizeY(); i++)
                    {
                        graph->addTask(task(this, &voxelSpace::buildSlice), nullptr, chunk.second, i, noiseSurface);
                    }
            }

        graph->execute();
        graph->clear(); 

        unsigned int offset = 0;
        for (auto &chunk : m_loadedChunks)
            {
                chunk.second.m_indexOffset = offset;
                buildChunk(chunk.second, offset);
            }
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
        OPTICK_EVENT();
        constexpr float gridOffset = 0.000001f;

        glm::vec3 worldPosition = getModelTransformation() * glm::vec4(origin, 0.f);
        glm::ivec3 chunkPosition{};
        glm::vec3 localPosition{};

        glm::ivec3 gridPosition = glm::round(worldPosition / c_voxelSize);
        glm::vec3 deltaDistance = glm::abs(1.f / direction);
        glm::vec3 sideDistance;
        glm::ivec3 step;

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
        const int maxStepCount = glm::length(static_cast<glm::vec3>(c_chunkSize)) * m_loadedChunks.size();
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

                transformSpace(gridPosition, chunkPosition, localPosition);
                if (m_loadedChunks.find(chunkPosition) != m_loadedChunks.end())
                    {
                        voxelType type = m_loadedChunks.at(chunkPosition).m_chunk.at(localPosition.x, localPosition.y, localPosition.z);
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
        for (auto &chunk : m_loadedChunks)
            {
                vertexCount += chunk.second.m_vertexCount;
            }

        return vertexCount;
    }

unsigned int voxelSpace::getIndexCount() const
    {
        unsigned int indexCount = 0;
        for (auto &chunk : m_loadedChunks)
            {
                indexCount += chunk.second.m_indexCount;
            }

        return indexCount;
    }

const voxelType &voxelSpace::at(glm::vec3 position) const
    {
        OPTICK_EVENT();
        glm::ivec3 chunkPosition{};
        glm::vec3 localPosition{};
        transformSpace(position, chunkPosition, localPosition);
        assert(m_loadedChunks.find(chunkPosition) != m_loadedChunks.end());
        return m_loadedChunks.at(chunkPosition).m_chunk.at(localPosition.x, localPosition.y, localPosition.z);
    }

void voxelSpace::setAt(glm::vec3 position, voxelType type)
    {
        OPTICK_EVENT();
        glm::ivec3 chunkPosition{};
        glm::vec3 localPosition{};
        transformSpace(position, chunkPosition, localPosition);
        assert(m_loadedChunks.find(chunkPosition) != m_loadedChunks.end());
        chunkData &chunk = m_loadedChunks.at(chunkPosition);
        chunk.m_chunk.at(localPosition.x, localPosition.y, localPosition.z) = type;

        voxelChunk::sizeType subChunkCountX = static_cast<voxelChunk::sizeType>(std::ceil(static_cast<float>(chunk.m_sizeX) / c_chunkSubSize));
        voxelChunk::sizeType subChunkCountY = static_cast<voxelChunk::sizeType>(std::ceil(static_cast<float>(chunk.m_sizeY) / c_chunkSubSize));
        voxelChunk::sizeType subChunkCountZ = static_cast<voxelChunk::sizeType>(std::ceil(static_cast<float>(chunk.m_sizeZ) / c_chunkSubSize));

        unsigned int subChunkX = localPosition.x / c_chunkSubSize;
        unsigned int subChunkY = localPosition.y / c_chunkSubSize;
        unsigned int subChunkZ = localPosition.z / c_chunkSubSize;
        int index = subChunkX + subChunkCountX * (subChunkY + subChunkCountY * subChunkZ);
        chunkVoxelData &subChunk = chunk.m_voxelData[index];

        chunk.m_vertexCount -= subChunk.m_vertexCount;
        chunk.m_indexCount -= subChunk.m_indexCount;

        voxelChunk::sizeType x = subChunkX * chunk.m_subSize;
        voxelChunk::sizeType y = subChunkY * chunk.m_subSize;
        voxelChunk::sizeType z = subChunkZ * chunk.m_subSize;

        unsigned int offset = chunk.m_indexOffset;
        for (int i = 0; i < index; i++)
            {
                offset += chunk.m_voxelData[i].m_vertexCount;
            }

        buildChunkMesh(chunk, subChunk, x, y, z);
        buildGeometry({ chunk.m_positionX, chunk.m_positionY, chunk.m_positionZ }, subChunk, offset);

        chunk.m_vertexCount += subChunk.m_vertexCount;
        chunk.m_indexCount += subChunk.m_indexCount;
        updateSubChunkMemory(subChunk);
    }

void buildChunkMesh(const voxelSpace::chunkData &chunkData, voxelSpace::chunkVoxelData &voxelData, voxelChunk::sizeType &x, voxelChunk::sizeType &y, voxelChunk::sizeType &z)
    {
        OPTICK_EVENT("buildChunkMesh - sub-chunk");
        voxelData.m_quads.clear();
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
        if (x >= chunkData.m_sizeX)
            {
                x = 0;
                y += chunkData.m_subSize;
                if (y >= chunkData.m_sizeY)
                    {
                        y = 0;
                        z += chunkData.m_subSize;
                    }
            }   
    }

void buildChunkMesh(voxelSpace::chunkData &chunkData)
    {
        OPTICK_EVENT();
        voxelChunk::sizeType x = 0;
        voxelChunk::sizeType y = 0;
        voxelChunk::sizeType z = 0;
        OPTICK_TAG("VoxelDataSize", chunkData.m_voxelData.size());
        OPTICK_TAG("VoxelSubSize", chunkData.m_subSize);
        for (auto &voxelData : chunkData.m_voxelData)
            {
                buildChunkMesh(chunkData, voxelData, x, y, z);
            }
    }

unsigned int buildGeometry(glm::vec3 offset, voxelSpace::chunkVoxelData &voxelData, unsigned int indexOffset)
    {
        OPTICK_EVENT();
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

        {
            OPTICK_EVENT("Resize Vertices/Indices");
            std::size_t expectedNewVertexSize = voxelData.m_quads.size() * 4;
            std::size_t expectedNewIndexSize = voxelData.m_quads.size() * 6;

            if (voxelData.m_vertices.size() < expectedNewVertexSize)
                {
                    voxelData.m_vertices.resize(expectedNewVertexSize);
                }

            if (voxelData.m_indices.size() < expectedNewIndexSize)
                {
                    voxelData.m_indices.resize(expectedNewIndexSize);
                }

            voxelData.m_indexCount = expectedNewIndexSize;
            voxelData.m_vertexCount = expectedNewVertexSize;
        }

        unsigned int vertexIndex = 0;
        // index of the current polygon index. Confusing, I know...
        unsigned int indexIndex = 0;

        OPTICK_TAG("VoxelDataQuads", voxelData.m_quads.size());
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

                v0.m_position += offset;
                v1.m_position += offset;
                v2.m_position += offset;
                v3.m_position += offset;
                
                {
                    voxelData.m_vertices[vertexIndex + 0] = vertexArr[0];
                    voxelData.m_vertices[vertexIndex + 1] = vertexArr[1];
                    voxelData.m_vertices[vertexIndex + 2] = vertexArr[2];
                    voxelData.m_vertices[vertexIndex + 3] = vertexArr[3];
                    if (quad.m_orientation < 0)
                        {
                            voxelData.m_indices[indexIndex + 0] = indexArrNegative[0];
                            voxelData.m_indices[indexIndex + 1] = indexArrNegative[1];
                            voxelData.m_indices[indexIndex + 2] = indexArrNegative[2];
                            voxelData.m_indices[indexIndex + 3] = indexArrNegative[3];
                            voxelData.m_indices[indexIndex + 4] = indexArrNegative[4];
                            voxelData.m_indices[indexIndex + 5] = indexArrNegative[5];
                        }
                    else 
                        {
                            voxelData.m_indices[indexIndex + 0] = indexArrPositive[0];
                            voxelData.m_indices[indexIndex + 1] = indexArrPositive[1];
                            voxelData.m_indices[indexIndex + 2] = indexArrPositive[2];
                            voxelData.m_indices[indexIndex + 3] = indexArrPositive[3];
                            voxelData.m_indices[indexIndex + 4] = indexArrPositive[4];
                            voxelData.m_indices[indexIndex + 5] = indexArrPositive[5];
                        }
                    vertexIndex += 4;
                    indexIndex += 6;
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

        return indexOffset + voxelData.m_vertexCount;
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
