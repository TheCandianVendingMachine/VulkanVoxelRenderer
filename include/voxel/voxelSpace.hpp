// voxelSpace.hpp
// Defines a space for a voxel subset.
#pragma once
#include "voxel/voxelChunk.hpp"
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/vertexBuffer.hpp"
#include "graphics/indexBuffer.hpp"
#include "graphics/quad.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

class descriptorSet;
class voxelSpace
    {
        private:
            struct chunkVoxelData
                {
                    std::vector<quad> m_quads;
                    std::vector<vertex> m_vertices;
                    std::vector<fe::index> m_indices;
                };

            struct chunkData
                {
                    voxelChunk m_chunk;
                    std::vector<chunkVoxelData> m_voxelData;
                    voxelChunk::sizeType m_sizeX = 0;
                    voxelChunk::sizeType m_sizeY = 0;
                    voxelChunk::sizeType m_sizeZ = 0;
                    voxelChunk::sizeType m_subSize = 0;
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

            static constexpr voxelChunk::sizeType c_chunkSubSize = 10;
            localBuffer m_localBuffer;
            chunkData m_testChunk;

            glm::mat4 m_translation;
            glm::quat m_quaternion;

            friend void buildChunkMesh(chunkData &chunkData);
            friend unsigned int buildGeometry(chunkVoxelData &voxelData, unsigned int indexOffset);

            void updateChunkMemory(chunkData &chunk, void *stagingBuffer, unsigned long long vertexBufferOffset);
            void updateMemory();

            void createChunk(chunkData &chunk, const voxelChunk::sizeType sizeX, const voxelChunk::sizeType sizeY, const voxelChunk::sizeType sizeZ);
            void buildChunk(chunkData &chunk);
            void destroyChunk(chunkData &chunk);

        public:
            voxelSpace() = default;
            ~voxelSpace();
            void create();
            void destroy();

            void createWorld();

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

    };
