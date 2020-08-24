// voxelSpace.hpp
// Defines a space for a voxel subset.
#pragma once
#include "voxel/voxelChunk.hpp"
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/vertexBuffer.hpp"
#include "graphics/indexBuffer.hpp"
#include "graphics/quad.hpp"
#include <glm/gtx/hash.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <unordered_map>

#include "FastNoise.h"

class descriptorSet;
class taskGraph;
class voxelSpace
    {
        private:
            struct chunkVoxelData
                {
                    std::vector<quad> m_quads;
                    std::vector<vertex> m_vertices;
                    std::vector<fe::index> m_indices;
                    void *m_vertexStagingBuffer = nullptr;
                    void *m_indexStagingBuffer = nullptr;
                    std::size_t m_vertexCount = 0;
                    std::size_t m_indexCount = 0;
                };

            struct chunkData
                {
                    voxelChunk m_chunk;
                    std::vector<chunkVoxelData> m_voxelData;
                    voxelChunk::sizeType m_sizeX = 0;
                    voxelChunk::sizeType m_sizeY = 0;
                    voxelChunk::sizeType m_sizeZ = 0;
                    voxelChunk::sizeType m_positionX = 0;
                    voxelChunk::sizeType m_positionY = 0;
                    voxelChunk::sizeType m_positionZ = 0;
                    voxelChunk::sizeType m_subSize = 0;
                    unsigned int m_indexOffset = 0;
                    unsigned int m_vertexCount = 0;
                    unsigned int m_indexCount = 0;
                };

            struct localBuffer
                {
                    vulkanBuffer m_masterBuffer;
                    vulkanBuffer m_masterStagingBuffer;
                    void *m_cpuStagingBuffer = nullptr;
                    unsigned int m_maxVertexCount = 0;
                    unsigned int m_maxIndexCount = 0;
                    unsigned int m_bufferSize = 0;
                    bool m_needUpdate = false;
                    bool m_exists = false;

                    void create(unsigned int vertexCount, unsigned int indexCount);
                    void destroy();
                };

            static constexpr voxelChunk::sizeType c_chunkSubSize = 32;
            static constexpr glm::ivec3 c_chunkSize = { 64, 32, 64 };
            static constexpr float c_voxelSize = 1.0f;
            localBuffer m_localBuffer;
            std::unordered_map<glm::ivec3, chunkData> m_loadedChunks;

            glm::mat4 m_translation;
            glm::quat m_quaternion;

            friend void buildChunkMesh(chunkData &chunkData);
            friend unsigned int buildGeometry(glm::vec3 offset, chunkVoxelData &voxelData, unsigned int indexOffset);

            void updateChunkMemory(chunkData &chunk);
            void updateChunkMemory(chunkData &chunk, void *stagingBuffer, unsigned long long vertexBufferOffset, int &vertexOffset, int &indexOffset);
            void updateMemory();

            void createChunk(chunkData &chunk, const voxelChunk::sizeType sizeX, const voxelChunk::sizeType sizeY, const voxelChunk::sizeType sizeZ, const voxelChunk::sizeType posX, const voxelChunk::sizeType posY, const voxelChunk::sizeType posZ);
            void buildChunk(chunkData &chunk, unsigned int &totalIndexOffset);
            void destroyChunk(chunkData &chunk);

            void buildSlice(chunkData &chunk, unsigned int y, const FastNoise &noise);
            void transformSpace(glm::vec3 globalPosition, glm::ivec3 &chunkPos, glm::vec3 &localPos) const;
            void transformChunkSpace(glm::vec3 globalPosition, glm::ivec3 &chunkPos) const;
            void transformLocalSpace(glm::vec3 globalPosition, glm::vec3 &localPos) const;

        public:
            voxelSpace() = default;
            ~voxelSpace();
            void create();
            void destroy();

            void createWorld(taskGraph *graph);

            glm::mat4 getModelTransformation() const;
            glm::vec<3, int> raycast(const glm::vec3 origin, const glm::vec3 direction);

            void updateBuffers(vulkanCommandBuffer &commandBuffer);
            bool needsUpdate() const;

            vulkanBuffer &getBufferMemory();
            VkDeviceSize getVertexMemoryOffset() const;
            VkDeviceSize getIndexBufferSize() const;
            VkDeviceSize getVertexBufferSize() const;
            unsigned int getVertexCount() const;
            unsigned int getIndexCount() const;

            const voxelType &at(glm::vec3 position) const;
            void setAt(glm::vec3 position, voxelType type);

    };
