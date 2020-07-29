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

    };
