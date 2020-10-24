// sparseVoxelOctree.hpp
// Represents voxels in 3d world in a easy to parse data format that can be sent to the GPU easily through a buffer
#pragma once
#include <cstdint>
#include <vector>
#include <memory>

class sparseVoxelOctree
    {
        public:
            /*
                Voxel is setup in memory as:
                    Occupied Children (8 Bits)  [Binary Mask]
                    Colour Red (5 bits)         [Number]
                    Colour Green (6 bits)       [Number]
                    Colour Blue (5 bits)        [Number]
                    24 Bits

            */
            union voxelNode
                {
                    std::int32_t entire;
                    struct {
                        unsigned char children : 8;
                        struct {
                            unsigned char r : 5;
                            unsigned char g : 6;
                            unsigned char b : 5;
                        } colour;
                    } data;
                };

            struct cpuNode
                {
                    std::uint64_t index = 0;
                    std::uint64_t nextSibling = 0;
                    std::uint64_t firstChild = 0;
                    voxelNode voxel;
                };
            static constexpr auto voxelNodeSize = sizeof(voxelNode);
            static constexpr auto cpuNodeSize = sizeof(cpuNode);

            unsigned int m_treeSize = 0;
            std::vector<cpuNode> m_cpuVoxels;

            cpuNode &createNode();

            void subdivide(cpuNode &node, unsigned char children, unsigned char *(childrenIndicesOfNode[8]) = nullptr);
            bool exists(double x, double y, double z, unsigned int depth, unsigned int *lastVoxelIndex = nullptr, unsigned int *lastDepth = nullptr) const;

            void covertFromRGB24ToRGB16(char &r, char &g, char &b);

        public:
            void addVoxel(double x, double y, double z, unsigned int depth, char r = 255, char g = 255, char b = 255);

            sparseVoxelOctree(unsigned int treeSize);
    };
