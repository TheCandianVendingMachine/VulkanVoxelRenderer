#include "voxel/voxelChunk.hpp"
#include <fstream>

voxelChunk::voxelChunk(voxelChunk::sizeType sizeX, voxelChunk::sizeType sizeY, voxelChunk::sizeType sizeZ)
    {
        create(sizeX, sizeY, sizeZ);
    }

void voxelChunk::create(voxelChunk::sizeType sizeX, voxelChunk::sizeType sizeY, voxelChunk::sizeType sizeZ)
    {
        m_voxels.resize(sizeX * sizeY * sizeZ);
        m_sizeX = sizeX;
        m_sizeY = sizeY;
        m_sizeZ = sizeZ;
        m_voxelData = m_voxels.data();
    }

const voxelType &voxelChunk::at(voxelChunk::sizeType x, voxelChunk::sizeType y, voxelChunk::sizeType z) const
    {
        return m_voxelData[x + m_sizeX * (y * m_sizeZ * z)];
    }

voxelType &voxelChunk::at(voxelChunk::sizeType x, voxelChunk::sizeType y, voxelChunk::sizeType z)
    {
        return m_voxelData[x + m_sizeX * (y * m_sizeZ * z)];
    }

const voxelType &voxelChunk::at(sizeType index) const
    {
        return m_voxelData[index];
    }

voxelType &voxelChunk::at(sizeType index)
    {
        return m_voxelData[index];
    }

const voxelType &voxelChunk::operator[](sizeType index) const
    {
        return at(index);
    }

voxelType &voxelChunk::operator[](sizeType index)
    {
        return at(index);
    }

voxelChunk::sizeType voxelChunk::getSizeX() const
    {
        return m_sizeX;
    }

voxelChunk::sizeType voxelChunk::getSizeY() const
    {
        return m_sizeY;
    }

voxelChunk::sizeType voxelChunk::getSizeZ() const
    {
        return m_sizeZ;
    }

voxelChunk::sizeType voxelChunk::getVolume() const
    {
        return m_sizeX * m_sizeY * m_sizeZ;
    }

void voxelChunk::write(const char *file)
    {
        std::vector<char> voxelBuffer(m_voxels.size() * sizeof(std::underlying_type<voxelType>::type));
        std::memcpy(voxelBuffer.data(), m_voxels.data(), m_voxels.size());

        std::ofstream out(file, std::ios::trunc | std::ios::binary);
        out.write(voxelBuffer.data(), voxelBuffer.size());
        out.close();
    }

void voxelChunk::read(const char *file)
    {
        std::ifstream in(file, std::ios::ate | std::ios::binary);
        std::size_t fileSize = static_cast<std::size_t>(in.tellg());
        std::vector<char> buffer(fileSize);
        in.seekg(0);
        in.read(buffer.data(), fileSize);
        in.close();

        m_voxels.resize(buffer.size() / sizeof(std::underlying_type<voxelType>::type));
        std::memcpy(m_voxels.data(), buffer.data(), buffer.size());
    }

void voxelChunk::mesh(std::vector<quad> &quads)
    {
        for (int x = 0; x < m_sizeX; x++)
            {
                for (int y = 0; y < m_sizeY; y++)
                    {
                        for (int z = 0; z < m_sizeZ; z++)
                            {
                                const float voxelSize = 1.f;

                                if (at(x, y, z) == voxelType::NONE) { continue; }
                                float posX = x * voxelSize;
                                float posY = y * voxelSize;
                                float posZ = z * voxelSize;

                                // although this may leave the bounds of the array it shouldn't be an issue. We are only reading not writing
                                voxelType neighbors[] = {
                                    at(x, y, z + 1),
                                    at(x, y, z - 1),
                                    at(x, y - 1, z),
                                    at(x, y + 1, z),
                                    at(x + 1, y, z),
                                    at(x - 1, y, z)
                                };

                                quad q = {};
                                q.m_size = { voxelSize, voxelSize };
                                // front/back plane
                                if ((z + 1) >= m_sizeZ || neighbors[0] == voxelType::NONE)
                                    {
                                        q.m_orientation = 1;
                                        q.m_position.x = posX;
                                        q.m_position.y = posY;
                                        q.m_position.z = posZ + voxelSize;
                                        quads.push_back(q);
                                    }

                                if (z <= 0 || neighbors[1] == voxelType::NONE)
                                    {
                                        q.m_orientation = -1;
                                        q.m_position.x = posX;
                                        q.m_position.y = posY;
                                        q.m_position.z = posZ;
                                        quads.push_back(q);
                                    }

                                // top/bottom plane
                                if (y <= 0 || neighbors[2] == voxelType::NONE)
                                    {
                                        q.m_orientation = 2;
                                        q.m_position.x = posX;
                                        q.m_position.y = posZ;
                                        q.m_position.z = posY;
                                        quads.push_back(q);
                                    }

                                if ((y + 1) >= m_sizeY || neighbors[3] == voxelType::NONE)
                                    {
                                        q.m_orientation = -2;
                                        q.m_position.x = posX;
                                        q.m_position.y = posZ;
                                        q.m_position.z = posY + voxelSize;
                                        quads.push_back(q);
                                    }

                                // left/right plane
                                if ((x + 1) >= m_sizeX || neighbors[4] == voxelType::NONE)
                                    {
                                        q.m_orientation = 3;
                                        q.m_position.x = posY;
                                        q.m_position.y = posZ;
                                        q.m_position.z = posX + voxelSize;
                                        quads.push_back(q);
                                    }

                                if (x <= 0 || neighbors[5] == voxelType::NONE)
                                    {
                                        q.m_orientation = -3;
                                        q.m_position.x = posY;
                                        q.m_position.y = posZ;
                                        q.m_position.z = posX;
                                        quads.push_back(q);
                                    }
                            }
                    }
            }
    }
