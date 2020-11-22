// voxelGrid.hpp
// Represents voxels within a grid. High memory usage, use only for small memory objects.
// Does not own a transform, or even scale. Just represents voxels in a grid
#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include "voxel/voxel.hpp"

class storageBuffer;
class vulkanImage;
class voxelGrid
    {
        private:
            using indexType = std::vector<voxel>::size_type;
            using sizeType = unsigned long long;
            using sizeTypeVec = glm::vec<3, sizeType>;

            std::vector<voxel> m_voxels;
            sizeTypeVec m_size;

            struct fileMetaData
                {
                    // This is unchangable meta data. Do not add or remove anything
                    const std::uint64_t version = 1;
                    const std::uint64_t voxelSize = sizeof(voxel);
                    const std::uint64_t headerSize = sizeof(fileMetaData);
                    static constexpr std::uint64_t c_headerMetadataSize = sizeof(std::uint64_t) + sizeof(std::uint64_t) + sizeof(std::uint64_t);
                    // Change anything beneath this
                    std::uint64_t voxelCount = 0;
                    sizeTypeVec size;
                };

            indexType convertPositionToIndex(sizeTypeVec position) const;

        public:
            voxelGrid() = default;
            voxelGrid(sizeTypeVec size);
            void init(sizeTypeVec size);

            void add(sizeTypeVec position, voxel voxel);
            void remove(sizeTypeVec position);

            // Map to GPU data structures
            // Input is a non-created storage buffer
            void mapToStorageBuffer(storageBuffer &buffer, storageBuffer &shadowBuffer);

            // IO
            void save(const char *file);
            void load(const char *file);

    };
