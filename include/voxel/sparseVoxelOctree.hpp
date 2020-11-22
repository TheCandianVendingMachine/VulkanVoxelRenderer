// sparseVoxelOctree.hpp
// Represents voxels in 3d world in a easy to parse data format that can be sent to the GPU easily through a buffer
#pragma once
#include "voxel/voxel.hpp"
#include <cstdint>
#include <vector>

#include <glm/vec3.hpp>

class storageBuffer;
class sparseVoxelOctree
    {
        public:
            /*
                Voxel is setup in memory as:
                    Occupied Children (8 Bits)  [Binary Mask]
                    Leaf Children (8 Bits)      [Binary Mask]
                    Colour Red (5 bits)         [Number]
                    Colour Green (6 bits)       [Number]
                    Colour Blue (5 bits)        [Number]
                    32 Bits

            */
            union voxelNode
                {
                    std::int32_t entire;
                    struct {
                        unsigned char children : 8;
                        unsigned char leaves : 8; // specifies whether each child is a leaf or not
                        voxel voxel;
                    } data;
                };

            struct cpuNode
                {
                    std::uint64_t index = 0;
                    std::uint64_t nextSibling = 0;
                    std::uint64_t firstChild = 0;
                    voxelNode voxel;
                };

            struct gpuNode
                {
                    std::uint32_t index = 0;
                    std::uint32_t nextSibling = 0;
                    std::uint32_t firstChild = 0;
                    voxelNode voxel;
                };

            struct fileMetaData
                {
                    // This is unchangable meta data. Do not add or remove anything
                    const std::uint64_t version = 1;
                    const std::uint64_t cpuNodeSize = sizeof(cpuNode);
                    const std::uint64_t headerSize = sizeof(fileMetaData);
                    static constexpr std::uint64_t c_headerMetadataSize = sizeof(std::uint64_t) + sizeof(std::uint64_t) + sizeof(std::uint64_t);
                    // Change anything beneath this
                    std::uint64_t voxelCount = 0;
                };

            static constexpr auto c_voxelNodeSize = sizeof(voxelNode);
            static constexpr auto c_cpuNodeSize = sizeof(cpuNode);
            static constexpr auto c_gpuNodeSize = sizeof(gpuNode);

            unsigned int m_treeSize = 0;
            bool m_cpuChanged = true;
            std::vector<cpuNode> m_cpuVoxels;
            std::vector<gpuNode> m_gpuVoxels;

            cpuNode &createNode();

            void subdivide(cpuNode &node, unsigned char children, unsigned char *(childrenIndicesOfNode[8]) = nullptr);
            bool exists(double x, double y, double z, unsigned int depth, unsigned int *lastVoxelIndex = nullptr, unsigned int *lastDepth = nullptr) const;

            void covertFromRGB24ToRGB16(char &r, char &g, char &b);

        public:
            void addVoxel(double x, double y, double z, unsigned int depth, char r = 255, char g = 255, char b = 255);

            // IO
            void save(const char *filepath);
            void load(const char *filepath);

            // Input is a non-created storage buffer
            void mapToStorageBuffer(storageBuffer &buffer);

            sparseVoxelOctree(unsigned int treeSize);
    };
