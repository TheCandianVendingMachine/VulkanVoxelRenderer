#include "voxel/sparseVoxelOctree.hpp"
#include "graphics/storageBuffer.hpp"
#include <fstream>

sparseVoxelOctree::cpuNode &sparseVoxelOctree::createNode()
    {
        unsigned int index = m_cpuVoxels.size();
        m_cpuVoxels.emplace_back();
        cpuNode &child = m_cpuVoxels.back();
        child.index = index;
        child.voxel.entire = 0;
        return child;
    }

void sparseVoxelOctree::subdivide(cpuNode &node, unsigned char children, unsigned char *childrenIndicesOfNode[8])
    {
        // 0,0,0 is the far away corner top left, 1,1,1 is closest corner bottom right
        unsigned int nodeIndex = node.index;
        if (node.voxel.data.children != 0)
            {
                // we have children, subdivide new ones
                unsigned char totalChildren = children ^ node.voxel.data.children;
                children = children & totalChildren;
                unsigned char oldChildren = node.voxel.data.children;

                // Setting up previous child indices to sort list
                unsigned int oldChildrenIndices[8] = {
                    0, 0, 0, 0, 0, 0, 0, 0
                };
                unsigned int newChildrenIndices[8] = {
                    0, 0, 0, 0, 0, 0, 0, 0
                };

                unsigned int lastChildIndex = node.firstChild;
                {
                    for (int i = 0; i < 8; i++)
                        {
                            bool occupied = static_cast<bool>(oldChildren & (1 << i));
                            if (occupied)
                                {
                                    oldChildrenIndices[i] = lastChildIndex;
                                    lastChildIndex = m_cpuVoxels[lastChildIndex].nextSibling;
                                }
                        }
                }

                // Add children to node
                {
                    for (unsigned char i = 0; i < 8; i++)
                        {
                            bool occupied = static_cast<bool>(children & (1 << i));
                            if (occupied)
                                {
                                    m_cpuVoxels[nodeIndex].voxel.data.leaves |= (1 << i);
                                    newChildrenIndices[i] = createNode().index;
                                }
                        }
                }

                // Setup new list order. This is important since we assume the children to be in proper order for calculations
                unsigned int previousChild = 0;
                {
                    for (int i = 0; i < 8; i++)
                        {
                            if (oldChildrenIndices[i] != 0)
                                {
                                    if (previousChild == 0)
                                        {
                                            previousChild = oldChildrenIndices[i];
                                            m_cpuVoxels[nodeIndex].firstChild = previousChild;
                                        }
                                    else
                                        {
                                            m_cpuVoxels[previousChild].nextSibling = oldChildrenIndices[i];
                                            previousChild = oldChildrenIndices[i];
                                        }
                                }
                            else if (newChildrenIndices[i] != 0)
                                {
                                    if (previousChild == 0)
                                        {
                                            previousChild = newChildrenIndices[i];
                                            m_cpuVoxels[nodeIndex].firstChild = previousChild;
                                        }
                                    else
                                        {
                                            m_cpuVoxels[previousChild].nextSibling = newChildrenIndices[i];
                                            previousChild = newChildrenIndices[i];
                                        }
                                }
                        }
                }
            }
        else
            {
                unsigned int parentIndex = node.index;
                unsigned int currentIndex = parentIndex;

                for (unsigned char i = 0; i < 8; i++)
                    {
                        bool occupied = static_cast<bool>(children & (1 << i));
                        if (occupied)
                            {
                                m_cpuVoxels[nodeIndex].voxel.data.leaves |= (1 << i);

                                unsigned int index = createNode().index;

                                if (m_cpuVoxels[parentIndex].firstChild == 0)
                                    {
                                        m_cpuVoxels[parentIndex].firstChild = m_cpuVoxels.size() - 1;
                                    }

                                if (currentIndex != parentIndex)
                                    {
                                        m_cpuVoxels[currentIndex].nextSibling = index;
                                    }
                                currentIndex = index;
                            }
                    }
            }
        m_cpuVoxels[nodeIndex].voxel.data.children = m_cpuVoxels[nodeIndex].voxel.data.children | children;

        if (childrenIndicesOfNode)
            {
                unsigned int child = m_cpuVoxels[nodeIndex].firstChild;
                for (int i = 0; i < 8 && child != 0; i++)
                    {
                        bool occupied = static_cast<bool>(m_cpuVoxels[nodeIndex].voxel.data.children & (1 << i));
                        if (occupied)
                            {
                                (*childrenIndicesOfNode)[i] = m_cpuVoxels[child].index;
                                child = m_cpuVoxels[child].nextSibling;
                            }
                    }
            }
    }

bool sparseVoxelOctree::exists(double x, double y, double z, unsigned int depth, unsigned int *lastVoxelIndex, unsigned int *lastDepth) const
    {
        unsigned int workingIndex = 0;

        double minX = 0;
        double minY = 0;
        double minZ = 0;

        double maxX = m_treeSize;
        double maxY = m_treeSize;
        double maxZ = m_treeSize;

        unsigned int currentDepth = 0;
        while (currentDepth++ < depth)
            {
                unsigned char childIn = 0b000;

                bool xLM = x > (minX + (maxX / 2.0));
                bool yLM = y > (minY + (maxY / 2.0));
                bool zLM = z > (minZ + (maxZ / 2.0));

                childIn = childIn | (static_cast<unsigned char>(xLM) << 0);
                childIn = childIn | (static_cast<unsigned char>(yLM) << 1);
                childIn = childIn | (static_cast<unsigned char>(zLM) << 2);

                if (childIn & 0b001) { minX += maxX / 2; } else { maxX /= 2; }
                if (childIn & 0b010) { minY += maxY / 2; } else { maxY /= 2; }
                if (childIn & 0b100) { minZ += maxZ / 2; } else { maxZ /= 2; }

                if (!(m_cpuVoxels[workingIndex].voxel.data.children & (1 << childIn)))
                    {
                        if (lastVoxelIndex)
                            {
                                *lastVoxelIndex = workingIndex;
                            }
                        if (lastDepth)
                            {
                                *lastDepth = currentDepth - 1;
                            }
                        return false;
                    }

                unsigned int childIndices[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
                unsigned int child = m_cpuVoxels[workingIndex].firstChild;
                for (int i = 0; i <= childIn && child != 0; i++)
                    {
                        bool occupied = static_cast<bool>(m_cpuVoxels[workingIndex].voxel.data.children & (1 << i));
                        if (occupied)
                            {
                                childIndices[i] = m_cpuVoxels[child].index;
                                child = m_cpuVoxels[child].nextSibling;
                            }
                    }
                workingIndex = childIndices[childIn];
            }

        if (lastVoxelIndex)
            {
                *lastVoxelIndex = workingIndex;
            }
        if (lastDepth)
            {
                *lastDepth = currentDepth - 1;
            }
        return true;
    }

void sparseVoxelOctree::covertFromRGB24ToRGB16(char &r, char &g, char &b)
    {
        r = ((r >> 3) & 0b00011111);
        g = ((g >> 2) & 0b00111111);
        b = ((b >> 3) & 0b00011111);
    }

void sparseVoxelOctree::addVoxel(double x, double y, double z, unsigned int depth, char r, char g, char b)
    {
        m_cpuChanged = true;
        covertFromRGB24ToRGB16(r, g, b);

        unsigned int lastVoxel = 0;
        unsigned int lastDepth = 0;
        if (exists(x, y, z, depth, &lastVoxel, &lastDepth))
            {
                // change voxel colour
                m_cpuVoxels[lastVoxel].voxel.data.voxel.colour.r = r;
                m_cpuVoxels[lastVoxel].voxel.data.voxel.colour.g = g;
                m_cpuVoxels[lastVoxel].voxel.data.voxel.colour.b = b;
                return;
            }

        // subdivide and add voxel
        unsigned int minX = 0;
        unsigned int minY = 0;
        unsigned int minZ = 0;

        unsigned int maxX = m_treeSize;
        unsigned int maxY = m_treeSize;
        unsigned int maxZ = m_treeSize;

        unsigned int parent = 0;
        unsigned char lastChildIn = 0;

        for (int i = 0; i < depth; i++)
            {
                unsigned char childIn = 0b000;

                bool xLM = x > (minX + (maxX / 2.0));
                bool yLM = y > (minY + (maxY / 2.0));
                bool zLM = z > (minZ + (maxZ / 2.0));

                childIn = childIn | (static_cast<unsigned char>(xLM) << 0);
                childIn = childIn | (static_cast<unsigned char>(yLM) << 1);
                childIn = childIn | (static_cast<unsigned char>(zLM) << 2);

                if (childIn & 0b001) { minX += maxX / 2; } else { maxX /= 2; }
                if (childIn & 0b010) { minY += maxY / 2; } else { maxY /= 2; }
                if (childIn & 0b100) { minZ += maxZ / 2; } else { maxZ /= 2; }

                unsigned char children[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
                unsigned char *childrenPointer = children;

                if (i < lastDepth)
                    {
                        continue;
                    }
                if (i != 0)
                    {
                        m_cpuVoxels[parent].voxel.data.leaves ^= (1 << lastChildIn);
                    }
                subdivide(m_cpuVoxels[lastVoxel], 0b0000'0000 | (1 << childIn), &childrenPointer);
                parent = lastVoxel;
                lastChildIn = childIn;
                lastVoxel = children[childIn];
            }

        m_cpuVoxels[lastVoxel].voxel.data.voxel.colour.r = r;
        m_cpuVoxels[lastVoxel].voxel.data.voxel.colour.g = g;
        m_cpuVoxels[lastVoxel].voxel.data.voxel.colour.b = b;
    }

void sparseVoxelOctree::save(const char *filepath)
    {
        fileMetaData data;
        data.voxelCount = m_cpuVoxels.size();

        std::ofstream out(filepath, std::ios::binary);
        out.write(static_cast<const char*>(static_cast<void*>(&data)), sizeof(data));
        out.write(static_cast<const char*>(static_cast<void*>(m_cpuVoxels.data())), m_cpuVoxels.size() * sizeof(cpuNode));
        out.close();
    }

void sparseVoxelOctree::load(const char *filepath)
    {
        std::ifstream in(filepath, std::ios::ate | std::ios::binary);
        if (!in.is_open())
            {
                // <error>
                return;
            }
        std::streampos fileSize = in.tellg();
        std::vector<char> buffer(static_cast<std::size_t>(fileSize));

        fileMetaData data;

        in.seekg(0);

        char *dataPtr = reinterpret_cast<char*>(&data);
        in.read(dataPtr, fileMetaData::c_headerMetadataSize);
        switch (data.version)
            {
                case 0:
                    // <error>
                    return;
                    break;
                case 1:
                    in.read(dataPtr + fileMetaData::c_headerMetadataSize, data.headerSize - fileMetaData::c_headerMetadataSize);
                    in.read(buffer.data(), fileSize);
                    
                    m_cpuVoxels.clear();
                    m_cpuVoxels.resize(data.voxelCount);
                    for (std::size_t i = 0; i < m_cpuVoxels.size(); i++)
                        {
                            m_cpuVoxels[i] = *reinterpret_cast<cpuNode*>(buffer.data() + (i * data.cpuNodeSize));
                        }
                    break;
                default:
                    // <error>
                    return;
                    break;
            }
        in.close();
        m_cpuChanged = true;
    }

void sparseVoxelOctree::mapToStorageBuffer(storageBuffer &buffer)
    {
        if (m_cpuChanged)
            {
                m_gpuVoxels.clear();
                m_gpuVoxels.resize(m_cpuVoxels.size());

                unsigned int i = 0;
                for (const auto &cpuNode : m_cpuVoxels)
                    {
                        m_gpuVoxels[i].index = m_cpuVoxels[i].index;
                        m_gpuVoxels[i].nextSibling = m_cpuVoxels[i].nextSibling;
                        m_gpuVoxels[i].firstChild = m_cpuVoxels[i].firstChild;
                        m_gpuVoxels[i].voxel.entire = m_cpuVoxels[i].voxel.entire;
                        i++;
                    }
                m_cpuChanged = false;
            }
        if (buffer.getBufferSize() <= 0)
            {
                buffer.create(m_gpuVoxels.size(), c_gpuNodeSize);
            }

        buffer.bind(m_gpuVoxels.data());
    }

sparseVoxelOctree::sparseVoxelOctree(unsigned int treeSize) :
    m_treeSize(treeSize)
    {
        createNode();
    }
