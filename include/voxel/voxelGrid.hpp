// voxelGrid.hpp
// Represents voxels within a grid. High memory usage, use only for small memory objects.
// Does not own a transform, or even scale. Just represents voxels in a grid
#pragma once
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"
#include "voxel/voxel.hpp"

#include "sparseVoxelOctree.hpp"

class storageBuffer;
class vulkanImage;
class renderer;
class voxelGrid
    {
        public:
            using indexType = std::vector<voxel>::size_type;
            using sizeType = unsigned long long;
            using sizeTypeVec = glm::vec<3, sizeType>;

            struct floatVoxel
                {
                    unsigned short rgb : 16;
                };

            struct floatVoxelBinary
                {
                    unsigned char exists : 8;
                    unsigned char distance : 8;
                };

        private:
            std::vector<voxel> m_voxels;
            sizeTypeVec m_size;

            static constexpr unsigned int c_maxDepth = 7;
            sparseVoxelOctree m_octree;

            std::vector<floatVoxel> m_gpuVoxels;
            std::vector<floatVoxelBinary> m_gpuVoxelData;

            alignas(16) vulkanImage m_gridImage;
            vulkanImageView m_gridView;
            vulkanSampler m_gridSampler;

            alignas(16) vulkanImage m_shadowGridImage;
            vulkanImageView m_shadowGridView;
            vulkanSampler m_shadowGridSampler;

            struct fileMetaData
                {
                    // This is unchangable meta data. Do not add or remove anything
                    const std::uint64_t version = 1;
                    const std::uint64_t voxelSize = sizeof(voxel);
                    const std::uint64_t gpuVoxelDataSize = sizeof(floatVoxelBinary);
                    const std::uint64_t gpuVoxelSize = sizeof(floatVoxel);
                    const std::uint64_t headerSize = sizeof(fileMetaData);
                    static constexpr std::uint64_t c_headerMetadataSize = sizeof(std::uint64_t) * 5;
                    // Change anything beneath this
                    std::uint64_t voxelCount = 0;
                    sizeTypeVec size;
                };

            constexpr indexType convertPositionToIndex(sizeTypeVec position) const;
            constexpr sizeTypeVec convertIndexToPosition(indexType index) const;

            friend class raytracer;

            void generateMipMap(unsigned int depth, renderer &renderer);

        public:
            voxelGrid(sizeTypeVec size, renderer &renderer);
            void init(renderer &renderer);
            void create(sizeTypeVec size, renderer &renderer);

            ~voxelGrid();
            void destroy();

            void add(sizeTypeVec position, voxel voxel);
            void remove(sizeTypeVec position);

            // Map to GPU data structures
            // Input is a non-created storage buffer
            void mapToStorageBuffer(storageBuffer &buffer, storageBuffer &shadowBuffer);

            // IO
            void save(const char *file);
            void load(const char *file);

            void bake();
            void bakeImage(renderer &renderer);

            // collision functions
            bool rayIntersects(glm::vec3 origin, glm::vec3 direction);

    };
