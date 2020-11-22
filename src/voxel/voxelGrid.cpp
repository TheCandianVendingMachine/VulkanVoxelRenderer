#include "voxel/voxelGrid.hpp"
#include "graphics/storageBuffer.hpp"
#include "graphics/vulkan/vulkanImage.hpp"
#include <fstream>

voxelGrid::indexType voxelGrid::convertPositionToIndex(sizeTypeVec position) const
    {
        return static_cast<voxelGrid::indexType>(position.x + m_size.x * (position.y + m_size.y * position.z));
    }

voxelGrid::voxelGrid(sizeTypeVec size)
    {
        init(size);
    }

void voxelGrid::init(sizeTypeVec size)
    {
        m_size = size;
        m_voxels.resize(m_size.x * m_size.y * m_size.z);
    }

void voxelGrid::add(sizeTypeVec position, voxel voxel)
    {
        m_voxels[convertPositionToIndex(position)] = voxel;
    }

void voxelGrid::remove(sizeTypeVec position)
    {
        m_voxels[convertPositionToIndex(position)].entire = 0;
    }

void voxelGrid::mapToStorageBuffer(storageBuffer &buffer, storageBuffer &shadowBuffer)
    {
        struct floatVoxel
            {
                unsigned short rgb : 16;
            };

        struct floatVoxelBinary
            {
                unsigned char exists : 8;
            };

        {
            std::vector<floatVoxel> floatVoxels;
            floatVoxels.resize(m_voxels.size());

            int i = 0;
            for (const auto &voxel : m_voxels)
                {
                    // assuming VK_FORMAT_R5G6B5_UNORM_PACK16
                    if (voxel.entire != 0)
                        {
                            floatVoxels[i].rgb = voxel.colour.r | (voxel.colour.g << 5) | (voxel.colour.b << 11);
                        }
                    i++;
                }

            if (buffer.getBufferSize() <= 0)
                {
                    buffer.create(floatVoxels.size(), sizeof(floatVoxel));
                }
            buffer.bind(floatVoxels.data());
        }

        {
            std::vector<floatVoxelBinary> floatBinaryVoxels;
            floatBinaryVoxels.resize(m_voxels.size());
            int i = 0;
            for (const auto &voxel : m_voxels)
                {   
                    // assuming VK_FORMAT_R8_UNORM
                    // this doesnt work right now, isnt the same as the main voxel buffer
                    if (voxel.entire != 0)
                        {
                            floatBinaryVoxels[i].exists = 255;
                        }
                    i++;
                }

            if (shadowBuffer.getBufferSize() <= 0)
                {
                    shadowBuffer.create(floatBinaryVoxels.size(), sizeof(floatVoxelBinary));
                }
            shadowBuffer.bind(floatBinaryVoxels.data());
        }
    }

void voxelGrid::save(const char *file)
    {
        fileMetaData data;
        data.voxelCount = m_voxels.size();
        data.size = m_size;

        std::ofstream out(file, std::ios::binary);
        out.write(static_cast<const char*>(static_cast<void*>(&data)), sizeof(data));
        out.write(static_cast<const char*>(static_cast<void*>(m_voxels.data())), m_voxels.size() * sizeof(voxel));
        out.close();
    }

void voxelGrid::load(const char *file)
    {
        std::ifstream in(file, std::ios::ate | std::ios::binary);
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
                    
                    m_size = data.size;

                    m_voxels.clear();
                    m_voxels.resize(data.voxelCount);
                    for (std::size_t i = 0; i < m_voxels.size(); i++)
                        {
                            m_voxels[i] = *reinterpret_cast<voxel*>(buffer.data() + (i * data.voxelSize));
                        }
                    break;
                default:
                    // <error>
                    return;
                    break;
            }
        in.close();
    }
