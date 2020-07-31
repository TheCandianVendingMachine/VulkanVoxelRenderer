// voxelChunk.hpp
// A chunk of voxels. Designed to be quickly read to and from disk
#pragma once
#include <vector>
#include "voxel/voxelType.hpp"
#include "graphics/quad.hpp"

class voxelChunk
    {
        private:
            std::vector<voxelType> m_voxels;
            voxelType *m_voxelData = nullptr;

            using sizeType = std::vector<voxelType>::size_type;

            sizeType m_sizeX = 0;
            sizeType m_sizeY = 0;
            sizeType m_sizeZ = 0;
            float m_voxelSize = 1.f;

        public:
            voxelChunk() = default;
            voxelChunk(voxelChunk::sizeType sizeX, voxelChunk::sizeType sizeY, voxelChunk::sizeType sizeZ);
            void create(voxelChunk::sizeType sizeX, voxelChunk::sizeType sizeY, voxelChunk::sizeType sizeZ);

            const voxelType &at(voxelChunk::sizeType x, voxelChunk::sizeType y, voxelChunk::sizeType z) const;
            voxelType &at(voxelChunk::sizeType x, voxelChunk::sizeType y, voxelChunk::sizeType z);

            const voxelType &at(voxelChunk::sizeType index) const;
            voxelType &at(voxelChunk::sizeType index);

            const voxelType &operator[](voxelChunk::sizeType index) const;
            voxelType &operator[](voxelChunk::sizeType index);

            voxelChunk::sizeType getSizeX() const;
            voxelChunk::sizeType getSizeY() const;
            voxelChunk::sizeType getSizeZ() const;
            voxelChunk::sizeType getVolume() const;

            void write(const char *file);
            void read(const char *file);

            void setVoxelSize(float size);
            float getVoxelSize() const;

            void mesh(std::vector<quad> &quads);
            void meshAtPosition(std::vector<quad> &quads, unsigned int x, unsigned int y, unsigned int z);

            bool withinBounds(int x, int y, int z) const;

    };
