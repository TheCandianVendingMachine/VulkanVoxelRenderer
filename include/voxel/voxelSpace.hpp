// voxelSpace.hpp
// Defines a space for a voxel subset.
#pragma once
#include "voxel/voxelChunk.hpp"
#include "graphics/vertexBuffer.hpp"
#include "graphics/indexBuffer.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

class descriptorSet;
class voxelSpace
    {
        private:
            voxelChunk test;
            vertexBuffer m_vertexBuffer;
            indexBuffer m_indexBuffer;

            glm::mat4 m_translation;
            glm::quat m_quaternion;

            void buildGeometry(voxelChunk &chunk);

        public:
            voxelSpace() = default;
            ~voxelSpace();
            void create();
            void destroy();

            void createWorld();

            const vertexBuffer &getVertexBuffer() const;
            vertexBuffer &getVertexBuffer();

            const indexBuffer &getIndexBuffer() const;
            indexBuffer &getIndexBuffer();

            glm::mat4 getModelTransformation() const;

    };
