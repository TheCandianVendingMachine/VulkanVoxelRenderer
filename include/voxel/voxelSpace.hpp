// voxelSpace.hpp
// Defines a space for a voxel subset.
#pragma once
#include "voxel/voxelChunk.hpp"
#include "graphics/vertexBuffer.hpp"
#include "graphics/indexBuffer.hpp"

class voxelSpace
    {
        private:
            voxelChunk test;
            vertexBuffer m_vertexBuffer;
            indexBuffer m_indexBuffer;

            void buildGeometry(voxelChunk &chunk);

        public:
            voxelSpace();
            ~voxelSpace();
            void destroy();

            const vertexBuffer &getVertexBuffer() const;
            vertexBuffer &getVertexBuffer();

            const indexBuffer &getIndexBuffer() const;
            indexBuffer &getIndexBuffer();

    };
